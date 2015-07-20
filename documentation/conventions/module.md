
###Um die Struktur im Framework zu gewährleisten:
----------------------------------------------
Code:
- your module should be inside one namespace

Code-Dokumentation:
In jedes Modul:
Eine ReadMe.txt, in ihr steht:
- Wer hat es erstelllt
- Wann wurde es erstellt
- Was soll das Modul tun

Ein Modul-
Darf:
- Datenkanäle nutzen
- shared-core-Funktionen sowie shared-Modulfunktionen nutzen

Darf NICHT:
- Funktionen anderer Module aufrufen, welche nicht in shared liegen.
- Auf Module (im Sinne von laufenden Modulen) zugreifen

Soll:
- möglichst keine Funktionen, welche in anderen Modulen definiert sind benutzen. Es sei denn, es ist ein Utils-Modul. Dann ist dies erwünscht.

Ein RuntimeModul enthält die Methoden:
cycle() {hier kommt die logik rein}
getPriority() {wird zum sortieren der cycle-liste benutzt, um so höher, desto früher wird das modul aufgerufen}
init(){Hier werden die Datenkanäle angelegt}
dispose(){Freigeben aller Ressourcen}
