from sqlalchemy import create_engine
from sqlalchemy import and_
from sqlalchemy import insert
from sqlalchemy.orm import sessionmaker

from models import Color, State, Team, Player

engine = create_engine('postgresql://postgres:passw0rd@localhost:5432/ACC_BBALL')
session = sessionmaker(bind=engine)()

def purge_and_create_tables():
    '''create tables from model, drop first if exists
    
    '''
    if engine.dialect.has_table(engine.connect(), "player"):
        Player.__table__.drop(engine)
    if engine.dialect.has_table(engine.connect(), "team"):
        Team.__table__.drop(engine)
    if engine.dialect.has_table(engine.connect(), "state"):
        State.__table__.drop(engine)
    if engine.dialect.has_table(engine.connect(), "color"):
        Color.__table__.drop(engine)

    Color.__table__.create(engine)
    State.__table__.create(engine)
    Team.__table__.create(engine)
    Player.__table__.create(engine)
    return

def populate_table_color():
    with open('color.txt') as f:
        lines = f.readlines()
        for line in lines:
            line = line.replace("\n", "")
            attributes = line.split(" ")
            query = insert(Color).values(name=attributes[1])
            with engine.connect() as conn:
                conn.execute(query)
    return

def populate_table_state():
    with open('state.txt') as f:
        lines = f.readlines()
        for line in lines:
            line = line.replace("\n", "")
            attributes = line.split(" ")
            query = insert(State).values(name=attributes[1])
            with engine.connect() as conn:
                conn.execute(query)
    return

def populate_table_team():
    with open('team.txt') as f:
        lines = f.readlines()
        for line in lines:
            line = line.replace("\n", "")
            attributes = line.split(" ")
            query = insert(Team).values(
                name=attributes[1], 
                state_id=attributes[2], 
                color_id=attributes[3], 
                wins=attributes[4],
                losses=attributes[5]
            )
            with engine.connect() as conn:
                conn.execute(query)
    return

def populate_table_player():
    with open('player.txt') as f:
        lines = f.readlines()
        for line in lines:
            line = line.replace("\n", "")
            attributes = line.split(" ")
            query = insert(Player).values(
                team_id=attributes[1], 
                uniform_num=attributes[2], 
                first_name=attributes[3], 
                last_name=attributes[4],
                mpg=attributes[5],
                ppg=attributes[6],
                rpg=attributes[7],
                apg=attributes[8],
                spg=attributes[9],
                bpg=attributes[10]
            )
            with engine.connect() as conn:
                conn.execute(query)
    return

def populate_tables():
    populate_table_color()
    populate_table_state()
    populate_table_team()
    populate_table_player()
    return

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
			"{:.1f}".format(player.spg), "{:.1f}".format(player.bpg)
		)
	return 

def query2(team_color):
	color = session.query(Color).filter(Color.name == team_color)[0]
	teams = session.query(Team).filter(Team.color_id == color.color_id)

	print('NAME')
	for team in teams:
		print(team.name)
	return

def query3(team_name):
	team = session.query(Team).filter(Team.name == team_name)[0]
	players = session.query(Player).filter(Player.team_id == team.team_id).order_by(Player.ppg.desc())

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

	print('UNIFORM_NUM FIRST_NAME LAST_NAME')
	for player in players:
		print(player.uniform_num, player.first_name, player.last_name)
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

def try_run_testcases():
    query1(
        1, 33, 40,
        1, 15, 25,
        1, 3, 8,
        1, 0, 4,
        1, 0.3, 1.5, 
        1, 0.6, 2.2
    )
    query2('Orange')
    query3('VirginiaTech')
    query4("MA", "Maroon")
    query5(10)
    return 

if __name__ == '__main__':
    purge_and_create_tables()
    populate_tables()
    try_run_testcases()