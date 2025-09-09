# 🟦 Esercizio 3 – HC-SR04 in polling

## 🎯 Obiettivo

Leggere la distanza da un sensore **HC-SR04** con **Arduino UNO** usando **polling** (attesa attiva) e stampare i risultati sul **Monitor Seriale**. L’obiettivo è consolidare la generazione del burst `TRIG`, la misura della larghezza d’impulso su `ECHO` e la conversione tempo→distanza.

## 📚 Competenze sviluppate

* Gestione segnali digitali a tempo (TRIG/ECHO) con microsecondi.
* Implementazione di timeout robusti per evitare deadlock in busy-wait.
* Conversione da **pulse width** alla distanza (speed of sound).
* Strutturazione di una libreria a oggetti riusabile in ambiente Arduino.

## 🧱 Requisiti hardware

| Componente      | Q.tà | Note                     |
| --------------- | ---: | ------------------------ |
| Arduino UNO     |    1 | 5V logic, clock 16 MHz   |
| HC-SR04         |    1 | Vcc 5 V, GND, TRIG, ECHO |
| Cablaggi dupont |  \~4 | —                        |

> **Wiring consigliato (polling):** `TRIG → D9`, `ECHO → D8`.
> (Per la versione **interrupt** servirà `ECHO` su `D2` o `D3`, vedi Esercizio 3bis.)

## 💻 Requisiti software

* **Arduino IDE** (≥ 1.8.19 o IDE 2.x).
* Porta seriale a 115200 baud.

## 📦 File forniti

* `hcsr04.hpp` – classe `HCSR04`.
* `hcsr04.cpp` – implementazione.
* `Esercizio3.ino` – sketch esempio (polling).

## 🧪 Parametri e formule

* **Velocità del suono** (aria, 20 °C): *c* ≈ **343 m/s** ⇒ **0.0343 cm/μs**.
* Distanza (cm): `d = (pulse_us × 0.0343) / 2` = `pulse_us × 0.01715`.
* **Timeout consigliato**: 30 000 μs (copre \~5 m con margine).

## ✏️ Estensioni

* Media mobile su N letture.
* Soglia di validità e “Hold-Last-Value”.
* Compensazione temperatura (parametro opzionale alla classe).