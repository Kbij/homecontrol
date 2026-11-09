-- Geofencing feature: adds 5 nullable columns to HC_DB.Client to store each client's current
-- stationary geofence (NULL = no active geofence). Run this once against the HomeControl DB
-- before deploying the updated server build.

ALTER TABLE HC_DB.Client
    ADD COLUMN geofenceLat DOUBLE NULL,
    ADD COLUMN geofenceLon DOUBLE NULL,
    ADD COLUMN geofenceRadius DOUBLE NULL,
    ADD COLUMN geofenceCreatedAt DATETIME NULL,
    ADD COLUMN geofenceUpdatedAt DATETIME NULL;
