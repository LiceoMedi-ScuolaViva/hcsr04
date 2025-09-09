# 🟦 Esercizio 3 – HC-SR04 (driver a polling)

## 🎯 Obiettivo

Realizzare la misura della distanza con un sensore **HC-SR04** su **Arduino UNO** utilizzando un **driver a polling**, progettato in stile **MISRA C++**.
Lo scopo non è solo ottenere la distanza, ma anche consolidare le buone pratiche di progettazione software:

* generazione del burst `TRIG` con timing garantito,
* misura robusta della durata del segnale `ECHO`,
* gestione di timeout per evitare deadlock,
* conversione tempo → distanza con parametro configurabile (velocità del suono).

## 📚 Competenze sviluppate

* Astrazione a classi con interfaccia (`IHCSR04`) e implementazione concreta (`HCSR04_Polling`).
* Uso di busy-wait controllati con timeout.
* Parametrizzazione di driver embedded (pin, timeout, min cycle, sound speed).
* Rispetto di vincoli temporali in microsecondi tramite `micros()` e `delayMicroseconds()`.
* Stampa diagnostica e validazione dei codici di errore.

## 🧱 Requisiti hardware

| Componente      | Q.tà | Note                     |
| --------------- | ---: | ------------------------ |
| Arduino UNO     |    1 | logica 5 V, clock 16 MHz |
| HC-SR04         |    1 | Vcc 5 V, GND, TRIG, ECHO |
| Cablaggi dupont |  \~4 | —                        |

> **Wiring consigliato (polling):** `TRIG → D9`, `ECHO → D8`.

## 💻 Requisiti software

* **Arduino IDE** (≥ 1.8.19 o IDE 2.x).
* Porta seriale a **115200 baud**.

## 📦 Struttura dei file

* `hcsr04.hpp` – interfaccia astratta `IHCSR04` con configurazione e helpers.
* `hcsr04_polling.hpp / .cpp` – implementazione concreta **polling-based**.
* `Esercizio3.ino` – esempio minimale di utilizzo.

## 🧪 Parametri e formule

* **Velocità del suono** (aria, 20 °C): *c* ≈ **343 m/s** ⇒ **0.0343 cm/μs**.
* **Formula distanza (cm):**

  $$
  d = \frac{t_{\text{echo}} \times c}{2}
  $$

  es. `d = pulse_us × 0.01715` con c=343 m/s.
* **Timeout tipico:** 30 000 μs (\~5 m round-trip).
* **Idle minimo:** ≥ 60 000 μs per evitare echi multipli (datasheet).

## 🔧 Codici di stato (estratto)

* `HCSR04_OK` – misura valida.
* `HCSR04_ERR_TIMEOUT_ECHO_START` – nessun fronte di salita rilevato.
* `HCSR04_ERR_TIMEOUT_ECHO_END` – nessun fronte di discesa entro timeout.
* `HCSR04_ERR_BUSY` – tentativo di nuova misura troppo ravvicinato.
* `HCSR04_ERR_BAD_PARAM` – parametro non valido.

## ✏️ Estensioni suggerite

* Media mobile su N letture.
* Validazione con range min/max e modalità *Hold-Last-Value*.
* Driver **interrupt-based** (necessita `ECHO` su pin esterni INT: D2/D3).
* Compensazione dinamica della velocità del suono in base alla temperatura.