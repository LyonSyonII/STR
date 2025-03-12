# Earliest Deadline First

> El millor planificador

Es basa en el deadline més proper (Deadline absolut).  

El Deadline absolut és el temps des que el sistema ha començat.  
`Deadline absolut = Valor inicial + k * Periode + Deadline`

### Feasibility
#### Si Di = Ti
Utotal = Sumatori(Ci / Ti) <= 1

| Task | Computation Time | Deadline | T (Period) |
| ---- | ---------------- | -------- | ---------- |
| T1   | 2                | 6        | 6          | 
| T2   | 2                | 8        | 8          | 
| T3   | 3                | 10       | 10         | 

`Utotal = 2 / 6 + 2 / 8 + 3/10 = 0.88 <= 1` OK!

#### Si Di < Ti
Procesor demand criterion

| Task | Computation Time | Deadline | T (Period) |
| ---- | ---------------- | -------- | ---------- |
| T1   | 2                | 4        | 6          |
| T2   | 2                | 5        | 8          |
| T3   | 3                | 7        | 9          |

`Hiperperiode = mcm(6, 8, 9) = 72`

P fer l'analisi fins a temps `L*`:
`L* = Sumatori( (Ti - Di)*Ui ) / 1 - Utotal = ( (6 - 4)*3/6 + (8-5)*2/8 + (9-7)*3/9 ) / 1 - (2/6 + 2/8 + 3/9) = 25` 

Mirem tots els deadlines des de 0 fins a `L*` (25):
```
T1
0*6 + 4 = 4
1*6 + 4 = 10
2*6 + 4 = 16
3*6 + 4 = 22
4*6 + 4 = 28 < NO

T2
0*8 + 5 = 5
1*8 + 5 = 13
2*8 + 5 = 21

T3
0*9 + 7 = 7
1*9 + 7 = 16
2*9 + 7 = 25

D = { 4, 5, 7, 10, 13, 16, 21, 22, 25 }
```

Fem l'analisi als deadlines `D`:

```
g(0, L) = Sumatori( Floor( (L + Ti-Di)/Ti ) * Ci ) <= L

g(0, 4) = Floor( (4 + 6 - 4)/6 )*2 + Floor( (4 + 8 - 5)/8 )*2 + Floor( (4 + 9 - 7)/9 )*3 = 2 <= 4. OK!
g(0, 5) = Floor( (5 + 6 - 4)/6 )*2 + Floor( (5 + 8 - 5)/8 )*2 + Floor( (5 + 9 - 7)/9 )*3 = 4 <= 5. OK!
g(0, 7) = 7 <= 7
g(0, 10) = 9 <= 10
g(0, 13) = 11 <= 13
g(0, 16) = 16 <= 16
g(0, 21) = 18 <= 21
g(0, 22) = 20 <= 22
g(0, 25) = 23 <= 25

OK! 
```