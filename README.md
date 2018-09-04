# Tesi Laboratorio Altair

#### Gestione di un motore attraverso X-Nucleo-IHM07M1

* [Datasheet nucleo] - [X-Nucleo-IHM07M1](https://github.com/snukneLeo/Progetto-laboratorio-Cyberfisico/blob/master/en.DM00226187.pdf)

#### Note
Nella cartella denominata "X-Nucleo-IHM07M1/src" si trova il file main.cpp che contiene il codice sorgente per il controllo della scheda nucleo.
Inizialmente dopo alcuni test, sono riuscito a far funzionare il motore controllato dalla scheda.
Il passo successivo è stato quello di ottimizzare i 6-step (passaggio dalla scheda "grande" a quella "piccola"), in modo che alla fine la scheda riesca a controllare al meglio il motore. Ora il controllo del motore è molto più fluido e ha parecchia velocità.
Il motore in questione è: 17.5T trackstars brushless.

#### Gestione di un motore attraverso X-Nucleo-IHM08M1

* [Datasheet nucleo] - [X-Nucleo-IHM08M1](https://github.com/snukneLeo/Progetto-laboratorio-Cyberfisico/blob/master/en.DM00251088.pdf)

#### Note
Nella cartella denominata "X-Nucleo-IHM08M1/src" si trova il file main.cpp che contiene il codice sorgente per il controllo della scheda nucleo.
Dopo alcuni test il motore non è controllato a dovere come nella X-Nucleo-IHM07M1. La scheda X-Nucleo-IHM08M1 è molto più "potente" di quella precedente, ma causa dei disturbi e fa saltare alcuni step al motore. A causa i questa cosa non è possibile utilizzare al meglio questa scheda con questo motore. Ecco perchè per ora si adoperà la X-Nucleo-IHM07M1.

