# Basketball Statistics - Database Programming

- pqxx version: plain SQL interface implemented in C++
  - [stable pqxx version](pqxx_ubuntu18), stable version for autograder(Ubuntu 18.04)
  - [beta pqxx version](pqxx_ubuntu20), version for Ubuntu 20.04
- [SQLAlchemy version](orm), object relation mapping(orm) interface implemented in Python
## System Requirements
Ubuntu 18.04/20.04 with Python3.8+

**Warning:** autograder is configured with AWS EC2 running Ubuntu 18.04 AMI,
using prepared statements (e.g. `pqxx::worker::exec_prepared()`) may cause compilation error

## Development Setup
### Local PostgreSQL
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

### Python
1. go to **`orm/`** and create a Python virtual environment
```
cd orm
python3 -m venv venv
```

2. activate the virtual environment and download dependencies
```
source venv/bin/activate
pip install -r requirements.txt
```
3. (optional) deactivate virtual environment on exit
```
deactivate
```