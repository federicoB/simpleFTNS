# Simple Fault Tolerant Network System

This is a project started for the Computer Networking curse.

The aim is to create a client-server system that is fault tolerant, that means guarantee services even with the failure of some components.

In this particular case the goal of the system is to offer a service for storing key-value pairs.

### How fault tolerance is obtained
Briefly:
* The server crashes are covered by storing the key-value pairs in a database.
* The client crashes are covered by saving a token file locally and the client can after restore sessions.

The following images explain more extensively the steps.

When a client make its first connection:

![New session image](http://i.imgur.com/iYUc3SX.png "New session")

If the client crashes, at restart it can restore a previous session:

![Restore session image](http://i.imgur.com/IQfKbfZ.png "Restore session")

# Installation
1. Download or clone the git repository
2. SQLite3 is required for compiling the project. If you don't have it installed in your system (also the dev package), install with:

    ```shell        
    sudo apt-get install sqlite3 libsqlite3-dev
    ```
    
3. Compile the project using CMake. I suggest you to create a specific directory for it:

    ```shell
    mkdir build
    cd build
    cmake ..
    make
    ```
    
4. Create a SQLite3 db file called ```mainDB.sqlite```. You can use [DB Browser for sqlite](https://github.com/sqlitebrowser/sqlitebrowser) for that.
5. Create Session and Variables tables with the following queries:

    ```sql
    CREATE TABLE `Sessions` (
    	`ID`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
	    `NOTUSEDYET`	TEXT
    );
    CREATE TABLE `Variables` (
	    `SESSID`	INTEGER NOT NULL,
	    `Name`	INTEGER NOT NULL,
	    `Value`	INTEGER NOT NULL,
	    PRIMARY KEY(SESSID,Name),
	    FOREIGN KEY(`SESSID`) REFERENCES Sessions ( ID )
    );
    ```
6. Start server and then client. Server use port 60010 and client connects to localhost.

## License
This software is released under the GNU AFFERO GPL license.

A copy of the license is provided in the LICENSE file.
