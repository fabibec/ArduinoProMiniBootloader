# ArduinoProMiniBootloader

## Steps

- Fuses verändern (Low-aktiv bedeutet im Burnomat eine 0)
    - [ ] Bootsz1/2 auf 0 -> 2048 Bytes für Bootloader (0x3800 - 0x3FFF)
    - [ ] Bootrst auf 0 -> damit Controlle bei Reset in den Bootloader springt

- [ ] Umbiegen der Interruptvektoren über IVSEL (siehe snippets.c)
- [ ] USART Menu bauen das gewisse Zeit wartet und dann in Applikation springt
    - vorher IVSEL wieder ändern und dann einfacht einen (goto *0) machen
    - Timer so 5sec laufen, bis dahin muss der Nutzer p eingegeben haben um den Programmiervorgang zu starten
- [ ] Parsen und übersetzen der HEX-Dateien
    - [ ] Dateien mittels USART empfangen (am besten Page für Page und nach einer Page immer Xoff schicken)
    - [ ] Hex in binär konvertieren
    - [ ] Checksumme berechnen und prüfen
    - [ ] Flash beschreiben -> [avr/boot.h](https://www.nongnu.org/avr-libc/user-manual/group__avr__boot.html) lesen!
- [ ] Defaultzustand wiederherstellen 
    - [ ] USART zurücksetzen
    - [ ] Interrupts ausschalten
    - [ ] Ports zurücksetzen
- goto *0

