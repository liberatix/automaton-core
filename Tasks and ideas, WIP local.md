# Delete of nodes and use of free locations.
1. Deleted nodes should be backed up only when we create new node at their location.
2. At commit changes, deleted nodes get overwriten by nodes moved on top of them.
    Backup is not necessary because we are applying all changes.
3. At discard_changes, when we recover the ancestors of the nodes marked as free
    the path to them gets restored and we erase all marks to free locations.
