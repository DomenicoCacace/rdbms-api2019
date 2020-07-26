# Overview
This repository contains a naïve C implementation of a Relational DBMS, as required by the [project specifications](docs/ProvaFinale2019.pdf).  
The project is evaluated in terms of correctness (output results) and efficiency (response time and occupied memory) on [public](TestCases/) and private test cases; for this particular implementation, the final score is 29/30.

# Project Description
The project goal is to implement a mechanism to monitor relationships between entities, which may change over time. The mechanism has to monitor the following events:  
- A new entity begins to be monitored  
- A monitored entity stops to be monitored    
- A new relationship is established between two monitored entities  
- An existing relationship between two monitored entities ceases to exist  

The relations might not be symmetrical.  

When the system receives a specific command, the application returns, for each relationship, the entity "receiving" more relationships.

## Commands

The application receives commands from an input file. The possible commands are the following:

- <b>addent<id_ent></b>: adds an entity identified by <i>id_ent</i> to the monitored entities;
- <b>delent<id_ent></b>: removes the entity identified by <i>id_ent</i> from the monitored entities
- <b>addrel<id_orig><id_dest><id_rel></b>: adds a relationship, identified by <i>id_rel</i>, from the entity <i>id_orig</i> to the entity<i>id_dest</i>
- <b>delrel<id_orig><id_dest><id_rel></b>: removes a relationship, identified by <i>id_rel</i>, from the entity <i>id_orig</i> to the entity<i>id_dest</i>
- <b>report</b>: prints, for each relationship, the entities having the most entering relationships
- <b>end</b> signals the end of the input file


More information about the commands and the parameters syntax can be found [here](docs/ProvaFinale2019.pdf)


