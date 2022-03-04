# Basketball Statistics - Database Programming

## System Requirements
Ubuntu 20.04

## Development Setup
1. update system, install postgres dev tools and C++ API
```
sudo apt install postgresql postgresql-contrib libpqxx-dev
```

2. start postgres service
```
sudo service postgresql start
```

3. connect to postgres server and set up password 
```
sudo su - postgres
psql
ALTER USER postgres with encrypted password 'passw0rd';
\q
exit
```

4. edit **`pg_bha.conf`** to allow localhost password authentication, by replacing ~~`local all postgres peer`~~ with `local all postgres md5`
```
sudo nano /etc/postgresql/12/main/pg_hba.conf
``` 

5. restart postgres
```
sudo service postgresql restart
```

6. start postgres shell and create database
```
psql -U postgres
CREATE DATABASE "ACC_BBALL";
```

7. verify database creation and exit
```
\l
exit
```