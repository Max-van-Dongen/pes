# PES Project

## Pi1:
- **Websocket Server**
  - Bestandslocatie: `/home/exellar/socketserver.py`
- **I2C Master**
  - **Slave 0x12 kan ontvangen:**
    - `0x01` Interne led aan
    - `0x02` Interne led uit
    - `0x03` Open Deur 5s blocking

## Pi2:
- **Websocket Client**
  - Bestandslocatie: `/home/exellar/sockettotal.py`
- **Functies**
  - `LedOn`
  - `LedOff`
  - `OpenDoor`
- **I2C Master**
  - **Polling voor beweging aan de slave**
- **Slave 0x12 stuurt terug:**
  - `0x01` Beweging
  - `0x02` Geen beweging
