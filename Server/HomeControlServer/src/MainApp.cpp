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
#include "Comm/TemperatureSourceIf.h"
#include "DAL/TemperatureWriter.h"
#include "DAL/HomeControlDal.h"
#include "Logic/ObjectPrinter.h"
#include "Logic/TemperatureFilter.h"
#include "Logic/CommRouter.h"
#include "Logic/SystemClock.h"
#include "Logic/Timer.h"
#include "Logic/HeaterControl.h"
#include "Hw/I2C.h"
#include "Hw/RelaisDriver.h"
#include "DAL/ObjectWriter.h"
#include "Rest/CppRestServer.h"
#include "Rest/RestService.h"
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
DEFINE_string(serial, "", "Serial port to use"); ///dev/ttyUSB0
DEFINE_string(i2c, "/dev/i2c-1", "I2C port to use");
DEFINE_string(dbserver, "192.168.10.20", "Database server");

void signal_handler(int sig);
void daemonize();

std::condition_variable exitCv;
std::mutex exitMutex;
bool exitMain = false;
int pidFilehandle;
const uint8_t XBEE_CHANNEL = 0x17;
const uint8_t RELAIS_ADDRESS = 0x08;
const int RELAIS_TIMEOUT_SECONDS = 120; //Thermostats must send there temperature within this time

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

		DalNs::ObjectWriter* writer = new DalNs::ObjectWriter(FLAGS_dbserver, 33060, "HC_DB", "hc", "bugs bunny");
		DalNs::HomeControlDal* dal = new DalNs::HomeControlDal(FLAGS_dbserver, 33060, "HC_DB", "hc", "bugs bunny");
        RestServerNs::RestService* restService = new RestServerNs::RestService(dal);
        RestServerNs::CppRestServer* cppRestServer = new RestServerNs::CppRestServer(5836, restService);

		CommNs::SocketFactory* factory = new CommNs::SocketFactory;
		CommNs::Server* server = new CommNs::Server(factory, 5678, dal);

		server->registerCommListener(writer);
		HwNs::I2C* i2c = nullptr;
		HwNs::RelaisDriver* relais = nullptr;
		LogicNs::HeaterControl* heaterControl = nullptr;
		CommNs::Serial* serial = nullptr;
		CommNs::DMFrameProcessor* frameProcessor = nullptr;
		CommNs::DMComm* dmComm = nullptr;
		CommNs::TemperatureSourceIf* sensors = nullptr;
		LogicNs::SystemClock clock;

		LogicNs::Timer* timer = nullptr;
		LogicNs::TemperatureFilter* filter = nullptr;
		DalNs::TemperatureWriter* tempWriter = nullptr;
		LogicNs::CommRouter* commRouter = nullptr;

		if (FLAGS_serial != "" || FLAGS_simulate > 0)
		{
			if (FLAGS_simulate == 0)
			{
				LOG(INFO) << "Using I2C Bus for relais: " << FLAGS_i2c;
				i2c = new HwNs::I2C(FLAGS_i2c);
				relais = new HwNs::RelaisDriver(i2c, RELAIS_ADDRESS, RELAIS_TIMEOUT_SECONDS);
				heaterControl = new LogicNs::HeaterControl(relais);
				LOG(INFO) << "Using serial port: " << FLAGS_serial;
				serial = new CommNs::Serial(FLAGS_serial, 38400);
				serial->openSerial();
				frameProcessor = new CommNs::DMFrameProcessor(serial);
				dmComm = new CommNs::DMComm(frameProcessor, XBEE_CHANNEL, {0x12, 0x13});
				sensors = new CommNs::TemperatureSensors(dmComm);
			}
			else
			{
				LOG(INFO) << "Simulating with number of sensors: " << FLAGS_simulate;
				sensors = new CommNs::TemperatureSensorsSimulator(FLAGS_simulate);
			}

			timer = new LogicNs::Timer(clock, sensors);
			filter = new LogicNs::TemperatureFilter(sensors, 0.2);
			tempWriter = new DalNs::TemperatureWriter(filter);
			commRouter = new LogicNs::CommRouter(dal, server, filter, heaterControl);
		}
    	// Wait until application stopped by a signal handler
        std::unique_lock<std::mutex> lk(exitMutex);
        exitCv.wait(lk, []{return exitMain;});

        if (commRouter) delete commRouter;
        if (tempWriter) delete tempWriter;
        if (filter) delete filter;
        if (timer) delete timer;

        if (serial != nullptr)
        {
        	delete sensors;
        	delete dmComm;
        	delete frameProcessor;
        	delete serial;
        	delete heaterControl;
        	delete relais;
        	delete i2c;
        }

        server->unRegisterCommListener(writer);

        delete writer;
        delete server;
        delete cppRestServer;
        delete restService;
        delete dal;
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
