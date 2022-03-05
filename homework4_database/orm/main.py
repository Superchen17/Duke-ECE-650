from sqlalchemy import create_engine
from sqlalchemy import and_
from sqlalchemy.orm import sessionmaker

from models import Color, State, Team, Player

engine = create_engine('postgresql://postgres:passw0rd@localhost:5432/ACC_BBALL')
session = sessionmaker(bind=engine)()

def query1(
		use_mpg, min_mpg, max_mpg,
		use_ppg, min_ppg, max_ppg,
		use_rpg, min_rpg, max_rpg,
		use_apg, min_apg, max_apg,
		use_spg, min_spg, max_spg,
		use_bpg, min_bpg, max_bpg
		):

	players = session.query(Player)
	if(use_mpg):
		players = players.filter(and_(Player.mpg >= min_mpg, Player.mpg <= max_mpg))
	if(use_ppg):
		players = players.filter(and_(Player.ppg >= min_ppg, Player.ppg <= max_ppg))
	if(use_rpg):
		players = players.filter(and_(Player.rpg >= min_rpg, Player.rpg <= max_rpg))
	if(use_apg):
		players = players.filter(and_(Player.apg >= min_apg, Player.apg <= max_apg))
	if(use_spg):
		players = players.filter(and_(Player.spg >= min_spg, Player.spg <= max_spg))
	if(use_bpg):
		players = players.filter(and_(Player.bpg >= min_bpg, Player.bpg <= max_bpg))

	print('PLAYER_ID TEAM_ID UNIFORM_NUM FIRST_NAME LAST_NAME MPG PPG RPG APG SPG BPG')
	for player in players:
		print(player.player_id, player.team_id, player.uniform_num, 
			player.first_name, player.last_name, 
			player.mpg, player.ppg, player.rpg, player.apg,	
			player.spg, player.bpg
		)
	return 

def query2(team_color):
	color = session.query(Color).filter(Color.name == team_color)[0]
	teams = session.query(Team).filter(Team.color_id == color.color_id)

	print('Name')
	for team in teams:
		print(team.name)
	return

def query3(team_name):
	team = session.query(Team).filter(Team.name == team_name)[0]
	players = session.query(Player).filter(Player.team_id == team.team_id)

	print('FIRST_NAME LAST_NAME')
	for player in players:
		print(player.first_name, player.last_name)
	return

def query4(team_state, team_color):
	state = session.query(State).filter(State.name == team_state)[0]
	color = session.query(Color).filter(Color.name == team_color)[0]
	teams = session.query(Team).filter(
		and_(Team.state_id == state.state_id, Team.color_id == color.color_id)
	)

	teamIdsofState = [team.team_id for team in teams]
	players = session.query(Player).filter(Player.team_id.in_(teamIdsofState))	

	print('FIRST_NAME LAST_NAME UNIFORM_NUM')
	for player in players:
		print(player.first_name, player.last_name, player.uniform_num)
	return

def query5(num_wins):
	teams = session.query(Team).filter(Team.wins > num_wins)
	teamIdsofWins = [team.team_id for team in teams]
	players = session.query(Player).filter(Player.team_id.in_(teamIdsofWins))

	print('FIRST_NAME LAST_NAME NAME WINS')
	for player in players:
		playerTeam = session.query(Team).filter(Team.team_id == player.team_id)[0]
		print(player.first_name, player.last_name, playerTeam.name, playerTeam.wins)
	return

query1( 
    1, 35, 40, 
    0, 0, 0, 
    0, 5, 10, 
    0, 0, 0, 
    0, 0, 0, 
    0, 0, 0
  )
query2('Maroon')
query3('VirginiaTech')
query4("MA", "Maroon")
query5(10)
