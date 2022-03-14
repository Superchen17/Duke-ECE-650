#include <iostream>
#include <pqxx/pqxx>
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdlib.h>

#include "exerciser.h"

using namespace std;
using namespace pqxx;

void try_open_file(std::ifstream& file, std::string fileName){
  file.open(fileName, std::ifstream::in);
  if(!file.is_open()){
    throw CustomFileException("error: cannot open file " + fileName);
  }
}

void execute_query(std::string query, connection* C){
  pqxx::work worker(*C);
  worker.exec(query);
  worker.commit();
}

void initiate_db(std::string fileName, connection* C){
  std::ifstream file;

  try_open_file(file, fileName);

  std::stringstream ss;
  std::string line;
  while(std::getline(file, line)){
    ss << line << std::endl;
  }
  file.close();

  execute_query(ss.str(), C);
}

void populate_table_player(std::string fileName, connection* C){
  std::ifstream file;
  try_open_file(file, fileName);

  std::string line;
  int playerId, teamId, jerseyNum, mpg, ppg, rpg, apg;
  double spg, bpg;
  std::string firstName, lastName;

  while(std::getline(file, line)){
    std::stringstream ss;
    ss << line;
    ss >> playerId >> teamId >> jerseyNum >> firstName >> lastName >> 
      mpg >> ppg >> rpg >> apg >> spg >> bpg;
    add_player(C, teamId, jerseyNum, firstName, lastName, 
      mpg, ppg, rpg, apg, spg, bpg);
  }
  file.close();
  
}

void populate_table_team(std::string fileName, connection* C){
  std::ifstream file;
  try_open_file(file, fileName);

  std::string line;
  std::string name;
  int teamId, stateId, colorId, wins, losses;

  while(std::getline(file, line)){
    std::stringstream ss;
    ss << line;
    ss >> teamId >> name >> stateId >> colorId >> wins >> losses;
    add_team(C, name, stateId, colorId, wins, losses);
  }
  file.close();
}

void populate_table_color(std::string fileName, connection* C){  
  std::ifstream file;
  try_open_file(file, fileName);

  std::string line;
  std::string colorId, name;

  while(std::getline(file, line)){
    std::stringstream ss;
    ss << line;
    ss >> colorId >> name;
    add_color(C, name);
  }
  file.close();
}

void populate_table_state(std::string fileName, connection* C){
  std::ifstream file;
  try_open_file(file, fileName);

  std::string line;
  std::string stateId, name;

  while(std::getline(file, line)){
    std::stringstream ss;
    ss << line;
    ss >> stateId >> name;
    add_state(C, name);
  }
  file.close();
}

int main (int argc, char *argv[]) 
{

  //Allocate & initialize a Postgres connection object
  connection *C;

  try{
    //Establish a connection to the database
    //Parameters: database name, user name, user password
    C = new connection("dbname=ACC_BBALL user=postgres password=passw0rd");
    if (C->is_open()) {
      cout << "Opened database successfully: " << C->dbname() << endl;
    } else {
      cout << "Can't open database" << endl;
      return 1;
    }
  } catch (const std::exception &e){
    cerr << e.what() << std::endl;
    return 1;
  }


  //TODO: create PLAYER, TEAM, STATE, and COLOR tables in the ACC_BBALL database
  //      load each table with rows from the provided source txt files
  try{
    initiate_db("init_db.sql", C);
    populate_table_color("color.txt", C);
    populate_table_state("state.txt", C);
    populate_table_team("team.txt", C);
    populate_table_player("player.txt", C);
  }
  catch(CustomFileException& e){
    std::cerr << e.what() << std::endl;
    exit(EXIT_FAILURE);
  }


  exercise(C);


  //Close database connection
  C->disconnect();

  return 0;
}


