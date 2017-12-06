# Database Manager library using TinySQL.

### Steps to run ###
```sh
$ make
$ ./database-manager
```

### Storage manager changes ###
We have made 2 changes to storage manager library.
To apply them, in case you are planning to use the original storage manager code,
use the following command:
```sh
$ git am -3 ./*.patch
```

### Disk I/O statistics ###

Disk I/O statistics can be enabled by using the following command:
```sh
$ ./database-manager TinySQLCommands.txt --print-stats
```
