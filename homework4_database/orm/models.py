from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy import Column
from sqlalchemy import Integer, String, Float
from sqlalchemy import ForeignKey

Base = declarative_base()

class Color(Base):
    __tablename__ = 'color'

    color_id = Column(Integer, primary_key=True, autoincrement=True)
    name = Column(String, nullable=False)

    def __init__(self, color_id, name):
        self.color_id = color_id
        self.name = name

class State(Base):
    __tablename__ = 'state'

    state_id = Column(Integer, primary_key=True, autoincrement=True)
    name = Column(String, nullable=False)

    def __init__(self, state_id, name):
        self.state_id = state_id
        self.name = name
        
class Team(Base):
    __tablename__ = 'team'

    team_id = Column(Integer, primary_key=True, autoincrement=True)
    name = Column(String, nullable=False)
    state_id = Column(Integer, ForeignKey('state.state_id'), nullable=False)
    color_id = Column(Integer, ForeignKey('color.color_id'), nullable=False)
    wins = Column(Integer, nullable=False)
    losses = Column(Integer, nullable=False)

    def __init__(self, team_id, name, state_id, color_id, wins, losses):
        self.team_id = team_id
        self.name = name
        self.state_id = state_id
        self.color_id = color_id
        self.wins = wins
        self.losses = losses

class Player(Base):
    __tablename__ = 'player'

    player_id = Column(Integer, primary_key=True, autoincrement=True)
    team_id = Column(Integer, ForeignKey('team.team_id'), nullable=False)
    uniform_num = Column(Integer, nullable=False)
    first_name = Column(String, nullable=False)
    last_name = Column(String, nullable=False)
    mpg = Column(Integer, nullable=False)
    ppg = Column(Integer, nullable=False)
    rpg = Column(Integer, nullable=False)
    apg = Column(Integer, nullable=False)
    spg = Column(Float, nullable=False)
    bpg = Column(Float, nullable=False)

    def __init__(self, player_id, team_id, uniform_num, first_name, last_name, 
                mpg, ppg, rpg, apg, spg, bpg):
        self.player_id = player_id
        self.team_id = team_id
        self.uniform_num = uniform_num
        self.first_name = first_name
        self.last_name = last_name
        self.mpg = mpg
        self.ppg = ppg
        self.rpg = rpg
        self.apg = apg
        self.spg = spg
        self.bpg = bpg       
