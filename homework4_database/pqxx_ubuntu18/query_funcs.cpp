#include "query_funcs.h"

#include <sstream>
#include <map>
#include <utility>
#include <iomanip>

void display_query_results(pqxx::result& r, std::string fieldNames){
  std::cout << fieldNames << std::endl;
  for(auto row = r.begin(); row != r.end(); row++){
    for(auto field = row.begin(); field != row.end(); field++){
      std::cout << field->c_str() << " ";
    }
    std::cout << std::endl;
  }
}

void display_query1_results(pqxx::result& r, std::string fieldNames){
  std::cout << fieldNames << std::endl;
  for (auto row = r.begin(); row != r.end(); row++) {
    std::cout << row[0].as<int>() << " " 
      << row[1].as<int>() << " " 
      << row[2].as<int>() << " " 
      << row[3].as<string>() << " " 
      << row[4].as<string>() << " "
      << row[5].as<int>() << " " 
      << row[6].as<int>() << " " 
      << row[7].as<int>() << " " 
      << row[8].as<int>() << " " << fixed << setprecision(1)
      << row[9].as<double>() << " " 
      << row[10].as<double>() 
      << std::endl;
  }
}

pqxx::result run_static_prepared_statement(connection* C, std::string queryName, std::string queryString){
  pqxx::work worker(*C);
  pqxx::result r = worker.exec(queryString);
  worker.commit();
  return r;
}

void add_player(connection *C, int team_id, int jersey_num, string first_name, string last_name,
                int mpg, int ppg, int rpg, int apg, double spg, double bpg)
{
  std::stringstream ss;
  ss << "INSERT INTO player(team_id, uniform_num, first_name, last_name, mpg, ppg, rpg, apg, spg, bpg) VALUES (";
  ss << team_id << ", ";
  ss << jersey_num << ", ";
  ss << pqxx::work(*C).quote(first_name) << ", ";
  ss << pqxx::work(*C).quote(last_name) << ", ";
  ss << mpg << ", ";
  ss << ppg << ", ";
  ss << rpg << ", ";
  ss << apg << ", ";
  ss << spg << ", ";
  ss << bpg;
  ss << ")";

  run_static_prepared_statement(C, "", ss.str());
}

void add_team(connection *C, string name, int state_id, int color_id, int wins, int losses)
{
  std::stringstream ss;
  ss << "INSERT INTO TEAM(name, state_id, color_id, wins, losses) VALUES(";
  ss << pqxx::work(*C).quote(name) << ", ";
  ss << state_id << ", ";
  ss << color_id << ", ";
  ss << wins << ", ";
  ss << losses;
  ss << ")";

  run_static_prepared_statement(C, "", ss.str());
}

void add_state(connection *C, string name)
{
  std::stringstream ss;
  ss << "INSERT INTO STATE(name) VALUES(";
  ss << pqxx::work(*C).quote(name);
  ss << ")";

  run_static_prepared_statement(C, "", ss.str());
}

void add_color(connection *C, string name)
{
  std::stringstream ss;
  ss << "INSERT INTO COLOR(name) VALUES(";
  ss << pqxx::work(*C).quote(name);
  ss << ")";

  run_static_prepared_statement(C, "", ss.str());
}

void query1(connection *C,
	          int use_mpg, int min_mpg, int max_mpg,
            int use_ppg, int min_ppg, int max_ppg,
            int use_rpg, int min_rpg, int max_rpg,
            int use_apg, int min_apg, int max_apg,
            int use_spg, double min_spg, double max_spg,
            int use_bpg, double min_bpg, double max_bpg
){
  std::map<std::string, int> filterDomain = {
    {"mpg", use_mpg},
    {"ppg", use_ppg},
    {"rpg", use_rpg},
    {"apg", use_apg},
    {"spg", use_spg},
    {"bpg", use_bpg},
  };

  std::map<std::string, std::pair<double, double> > filterBounds = {
    {"mpg", std::make_pair(min_mpg, max_mpg)},
    {"ppg", std::make_pair(min_ppg, max_ppg)},
    {"rpg", std::make_pair(min_rpg, max_rpg)},
    {"apg", std::make_pair(min_apg, max_apg)},
    {"spg", std::make_pair(min_spg, max_spg)},
    {"bpg", std::make_pair(min_bpg, max_bpg)},
  };

  std::stringstream ss;
  ss << "SELECT *" << std::endl;
  ss << "FROM player" << std::endl;

  int counter = 0;
  for(auto const& domain: filterDomain){
    if(domain.second == 1){
      std::pair<double, double> bounds = filterBounds[domain.first];
      if(counter == 0){
        ss << "WHERE ";
      }
      else{
        ss << "AND ";
      }
      ss << domain.first << " BETWEEN " << bounds.first << " AND " << bounds.second << std::endl;
      counter++;
    }
  }

  pqxx::result r = run_static_prepared_statement(C, "query1", ss.str());
  std::string header = "PLAYER_ID TEAM_ID UNIFORM_NUM FIRST_NAME LAST_NAME MPG PPG RPG APG SPG BPG";
  display_query1_results(r, header);
}

void query2(connection *C, string team_color)
{
  std::stringstream ss;
  ss << "SELECT t.name" << std::endl;
  ss << "FROM team t" << std::endl;
  ss << "INNER JOIN color c" << std::endl;
  ss << "ON t.color_id = c.color_id" << std::endl;
  ss << "WHERE c.name = \'" << team_color << "\'" << std::endl;

  pqxx::result r = run_static_prepared_statement(C, "query2", ss.str());
  std::string header = "NAME";
  display_query_results(r, header);
}

void query3(connection *C, string team_name)
{
  std::stringstream ss;
  ss << "SELECT p.first_name, p.last_name" << std::endl;
  ss << "FROM player p" << std::endl;
  ss << "INNER JOIN team t" << std::endl;
  ss << "ON p.team_id = t.team_id" << std::endl;
  ss << "WHERE t.name = \'" << team_name << "\'" << std::endl;
  ss << "ORDER BY p.ppg desc" << std::endl;

  pqxx::result r = run_static_prepared_statement(C, "query3", ss.str());
  std::string header = "FIRST_NAME LAST_NAME";
  display_query_results(r, header);
}

void query4(connection *C, string team_state, string team_color)
{
  std::stringstream ss;
  ss << "SELECT p.first_name, p.last_name, p.uniform_num" << std::endl;
  ss << "FROM player p" << std::endl;
  ss << "INNER JOIN team t" << std::endl;
  ss << "ON p.team_id = t.team_id" << std::endl;
  ss << "INNER JOIN state s" << std::endl;
  ss << "on s.state_id = t.state_id" << std::endl;
  ss << "INNER JOIN color c" << std::endl;
  ss << "on c.color_id = t.color_id" << std::endl;
  ss << "WHERE s.name = \'" << team_state << "\'" << std::endl;
  ss << "AND c.name = \'" << team_color << "\'" << std::endl;

  pqxx::result r = run_static_prepared_statement(C, "query4", ss.str());
  std::string header = "FIRST_NAME LAST_NAME UNIFORM_NUM";
  display_query_results(r, header);
}

void query5(connection *C, int num_wins)
{
  std::stringstream ss;
  ss << "SELECT p.first_name, p.last_name, t.name, t.wins" << std::endl;
  ss << "FROM player p" << std::endl;
  ss << "INNER JOIN team t" << std::endl;
  ss << "ON p.team_id = t.team_id" << std::endl;
  ss << "WHERE t.wins > " << num_wins << std::endl;

  pqxx::result r = run_static_prepared_statement(C, "query5", ss.str());
  std::string header = "FIRST_NAME LAST_NAME NAME WINS";
  display_query_results(r, header);
}
