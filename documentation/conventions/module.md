
###Um die Struktur im Framework zu gewährleisten:
----------------------------------------------

Code-Dokumentation:
In jedes Modul:
*Eine ReadMe.txt, in ihr steht:
 * Wer hat es erstelllt
 * Wann wurde es erstellt
 * Was soll das Modul tun

Ein Modul-
Darf:
-Datenkanäle nutzen
-shared-core-Funktionen sowie shared-Modulfunktionen nutzen

Darf NICHT:
-Funktionen anderer Module aufrufen, welche nicht in shared liegen.
-Auf Module (im Sinne von laufenden Modulen) zugreifen

Soll:
-möglichst keine Funktionen, welche in anderen Modulen definiert sind benutzen. Es sei denn, es ist ein Utils-Modul. Dann ist dies erwünscht.

Ein RuntimeModul enthält die Methoden:
cycle() {hier kommt die logik rein}
getPriority() {wird zum sortieren der cycle-liste benutzt, um so höher, desto früher wird das modul aufgerufen}
init(){Hier werden die Datenkanäle angelegt}
dispose(){Freigeben aller Ressourcen}

Git
----
 * The master-branch should always be working.
 * Nichts auf master pushen, nichts unkompilierbares auf public
 * Branches für Funktionen (Features), nicht personen-bezogen.
 * D.h. eine branch Parken, Umfeldmodell etc. Wenn diese FUNKTIONIEREN werden sie mit master gemerged. Somit ist in master immer eine vollfunktionsfähige Version.
 * Das system kann für Unterpunkte äuquivalent übernommen werden. D.h. Parken -> schnelles parken als zweig (branch) -> Wenn schnelles parken funktioniert in parken mergen und wenn dort alles funktioniert in master mergen.

