# Servers

Els servidors agafen accions aperiòdiques i les converteixen en periòdiques.

### Background Scheduling
"Sense servidor", crida directament la tasca sense fer-la periòdica.

- Opció 1: Donar-li la prioritat més baixa, s'executa al _background_.
- Opció 2: Donar-li la prioritat més alta, així s'executarà sempre que pugui.

### Fixed Priorities
#### Polling Server
Té un _budget_ assignat, i s'executa seguint un temps de còmput màxim i un periode.  
Les tasques aperiòdiques corresponents s'executen fins que el _budget_ s'acabi, i no es recarrega fins el proper periode. 

#### Deferable Server
S'activa sempre, fins que gasta el _budget_.

### Dynamic Priorities
