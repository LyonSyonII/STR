# Lab 3 - Motor Position Control
> Com funciona una aplicacio realista amb FreeRTOS

## Encoder
Dos sensors efecte hall
> Quan passa una part metalica al costat genera un pols (ISR)

Quan mes rapid giri, més petits són els polsos.  
Ho agafem des del micro amb una interrupcio (flanc pujada/baixada pin del pols).

Quina senyal li ha de donar els interruptors és una tasca (PID).  
Farem passar la interrupció pel FreeRTOS.

## PID
ref = alternant entre -90 i 90 cada segon
error = ref - angleMesurat
P = Kp * error
I += Ki * Tpid * error
D = Kd * (error - last_error) / Tpid

u = P + I + D

Signe => Direccio
Modul => PWM

# TENIM ASSIGNAT EL MOTOR 2