/*
 * MainApp.cpp
 *
 *  Created on: Mar 28, 2016
 *      Author: koen
 */



#include "Comm/Server.h"
#include "Comm/SocketFactory.h"
#include "Comm/Serial.h"
#include "Comm/TemperatureSensors.h"
#include "Comm/TemperatureSensorsSimulator.h"
#include "Comm/DMFrameProcessor.h"
#include "Comm/DMComm.h"
#include "DAL/TemperatureWriter.h"
#include "DAL/HomeControlDal.h"
#include "Logic/ObjectPrinter.h"
#include "Logic/TemperatureFilter.h"
#include "Logic/CommRouter.h"
#include "DAL/ObjectWriter.h"
#include <string>
#include <glog/logging.h>
#include <gflags/gflags.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fstream>
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>

DEFINE_bool(daemon, false, "Run Server as Daemon");
DEFINE_string(pidfile,"","Pid file when running as Daemon");
DEFINE_int32(simulate, 0, "The number of simulation sensors");
DEFINE_string(serial, "/dev/ttyUSB0", "Serial port to use");

void signal_handler(int sig);
void daemonize();

std::condition_variable exitCv;
std::mutex exitMutex;
bool exitMain = false;
int pidFilehandle;

void signal_handler(int sig)
{
	switch(sig)
    {
		case SIGHUP:
			LOG(WARNING) << "Received SIGHUP signal.";
			break;
		case SIGINT:
		case SIGTERM:
		case SIGKILL:
			LOG(INFO) << "Application exiting";
			{
				std::lock_guard<std::mutex> lk(exitMutex);
				exitMain = true;
				exitCv.notify_one();
			}
			break;
		default:
			LOG(WARNING) << "Unhandled signal " << strsignal(sig);
			break;
    }
}

void registerSignals()
{
    struct sigaction newSigAction;
    sigset_t newSigSet;

    /* Set signal mask - signals we want to block */
    sigemptyset(&newSigSet);
    sigaddset(&newSigSet, SIGCHLD);  /* ignore child - i.e. we don't need to wait for it */
    sigaddset(&newSigSet, SIGTSTP);  /* ignore Tty stop signals */
    sigaddset(&newSigSet, SIGTTOU);  /* ignore Tty background writes */
    sigaddset(&newSigSet, SIGTTIN);  /* ignore Tty background reads */
    sigaddset(&newSigSet, SIGKILL);  /* Netbeans terminate process */
    sigprocmask(SIG_BLOCK, &newSigSet, NULL);   /* Block the above specified signals */

    /* Set up a signal handler */
    newSigAction.sa_handler = signal_handler;
    sigemptyset(&newSigAction.sa_mask);
    newSigAction.sa_flags = 0;

    /* Signals to handle */
    sigaction(SIGHUP, &newSigAction, NULL);     /* catch hangup signal */
    sigaction(SIGTERM, &newSigAction, NULL);    /* catch term signal */
    sigaction(SIGINT, &newSigAction, NULL);     /* catch interrupt signal */
}

void daemonize()
{
    int pid, sid, i;
    char str[10];
    /* Check if parent process id is set */
    if (getppid() == 1)
    {
        /* PPID exists, therefore we are already a daemon */
        return;
    }

    /* Fork*/
    pid = fork();

    if (pid < 0)
    {
        /* Could not fork */
        exit(EXIT_FAILURE);
    }
    if (pid > 0)
    {
        /* Child created ok, so exit parent process */
        exit(EXIT_SUCCESS);
    }

    /* Get a new process group */
    sid = setsid();

    if (sid < 0)
    {
        exit(EXIT_FAILURE);
    }

    /* close all descriptors */
    for (i = getdtablesize(); i >= 0; --i)
    {
        close(i);
    }

    /* Route I/O connections */
    /* Open STDIN */
    i = open("/dev/null", O_RDWR);
    /* STDOUT */
    if(dup(i)); // if; avoid warning regarging return value
    /* STDERR */
    if(dup(i));

//    chdir(rundir); /* change running directory */

    /* Ensure only one copy */
    pidFilehandle = open(FLAGS_pidfile.c_str(), O_RDWR|O_CREAT, 0600);

    if (pidFilehandle == -1 )
    {
        /* Couldn't open lock file */
    	LOG(ERROR) << "Could not open PID lock file " << FLAGS_pidfile << ", exiting";
        exit(EXIT_FAILURE);
    }

    /* Try to lock file */
    if (lockf(pidFilehandle,F_TLOCK,0) == -1)
    {
        /* Couldn't get lock on lock file */
    	LOG(WARNING) << "Could not lock PID lock file " << FLAGS_pidfile << ", exiting";
        exit(EXIT_FAILURE);
    }

    /* Get and format PID */
    sprintf(str,"%d\n",getpid());

    /* write pid to lockfile */
    if(write(pidFilehandle, str, strlen(str)));
}

int main (int argc, char* argv[])
{
	std::string usage("HomeControl Server App. Sample usage:\n");
	usage += argv[0];

	google::SetUsageMessage(usage);
	google::SetVersionString("1.0.0");
	google::ParseCommandLineFlags(&argc, &argv, true);
	if (FLAGS_daemon)
	{
		daemonize();
	}
	registerSignals();

    try
    {
		google::InitGoogleLogging("HC");

		LOG(INFO) << "Home Control ServerApp";
		LOG(INFO) << "======================";
		LOG(INFO) << "Build on: " << __DATE__ << ", " << __TIME__;
		DalNs::ObjectWriter* writer = new DalNs::ObjectWriter;
		CommNs::SocketFactory* factory = new CommNs::SocketFactory;
		CommNs::Server* server = new CommNs::Server(factory, 5678);

		server->registerCommListener(writer);

		CommNs::Serial* serial = nullptr;
		CommNs::DMFrameProcessor* frameProcessor = nullptr;
		CommNs::DMComm* dmComm = nullptr;
		CommNs::TemperatureSourceIf* sensors = nullptr;

		if (FLAGS_simulate == 0)
		{
			LOG(INFO) << "Using serial port: " << FLAGS_serial;
			serial = new CommNs::Serial(FLAGS_serial, 38400);
			serial->openSerial();
			frameProcessor = new CommNs::DMFrameProcessor(serial);
			dmComm = new CommNs::DMComm(frameProcessor, 0x0B, {0x12, 0x13});
			sensors = new CommNs::TemperatureSensors(dmComm);
		}
		else
		{
			LOG(INFO) << "Simulating with number of sensors: " << FLAGS_simulate;
			sensors = new CommNs::TemperatureSensorsSimulator(FLAGS_simulate);
		}
		DalNs::HomeControlDal* dal = new DalNs::HomeControlDal;
		LogicNs::TemperatureFilter* filter = new LogicNs::TemperatureFilter(sensors, 0.2);
		DalNs::TemperatureWriter* tempWriter = new DalNs::TemperatureWriter(filter);
		LogicNs::CommRouter* commRouter = new LogicNs::CommRouter(dal, server, filter, nullptr);

    	// Wait until application stopped by a signal handler
        std::unique_lock<std::mutex> lk(exitMutex);
        exitCv.wait(lk, []{return exitMain;});

        delete commRouter;
        delete tempWriter;
        delete filter;
        delete dal;

        if (serial != nullptr)
        {
        	delete sensors;
        	delete dmComm;
        	delete frameProcessor;
        	delete serial;



        }
        server->unRegisterCommListener(writer);

        delete writer;
        delete server;
        delete factory;
    }
	catch (std::exception& ex)
	{
		std::cerr << "Exception: " << ex.what() << std::endl;
		LOG(ERROR) << "Exception: " << ex.what();
	}

	LOG(INFO) << "Bye...";
	google::ShutdownGoogleLogging();
	google::ShutDownCommandLineFlags();
	return EXIT_SUCCESS;
}


