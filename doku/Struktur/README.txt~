LMS = lightweight modular system

Ziel:
-core wird opensource
--Man soll ihn auch für andere Dinge nutzen können.
--Die quadrocoptermenschen sollen ihn auch nutzen können.
-Modulare Plattform
-Core soll Plattformunabhängig sein
--Module können plattformabhängig sein
-Jede größere Funktionalität wird in ein eigenes Modul gesteckt

########################################
Allgemein:
Module dürfen nur auf shared-src zugreifen, sofern er nicht im eigenen Modul liegt.
Der shared-ordner in Modulen dient eigentlich nur der Bereitstellung von Datenkanälen. In manchen fällen kann man allerdings auch dort "spezifische utils" auslagern.

########################################
Core:
Enthält wirklich nur die core-funktionalität, core will nichts von utils wissen!

########################################
Module:
In sich geschlossenes System.

Arten von Modulen:
-RuntimeModul:
--Hat eine cycle Methode, verbindet funktionalitäten

-UtilsModul:
--Stellt Funktionalitäten bereit
--Hat keine lauffähige cycle-methode

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

########################################
README.txt 
Enthält:
-Header: welchen den Status des Moduls enthält:
--Stati: 
---work in progress (Modul erst im entstehen, Schnittstellen noch nicht klar und nicht getestet),
---stable (Modul wird weiterentwickelt, eine Backwards-Kompatibilität so gut es geht bereitgestellt wird),
---deprecated (Modul wird nicht mehr weiterentwickelt)
--Die beteiligten Entwickler, den Ersteller und bei großen Modulen den Hauptverantwortlichen
--Kurzbeschreibung des Moduls
-Längere Beschreibung des Moduls (Falls nötig - besser einfach machen)
-Die verwendeten Schnittstellen (Datenkanäle) und die Bereitgestellten
-Die TOTO-Liste für das Modul


########################################
CHANGELOG.txt
Sie dient anderen (nicht dem Ersteller) ihren code an das geupdatete Modul anzupassen. WICHTIG!
Enthält:
-Alle wichtigen Änderungen mit Datum und Ersteller.
-Vorallem Änderungen der Schnittstellen nach außen!
-Kleine Bugfixes kommen hier nicht rein! Wenn dann nur als kleines anhängsel, an eine größere Änderung.



########################################
Git:
-Jedes Modul ergibt einen Branch bei git
-Niemand darf auf master/core pushen
-Alle module "erben" von master/core
-der Branch zur core-weiterentwicklung dient nur zur Weiterentwicklung. Module werden nicht davon "geforked" oder wie man es nennt :D
-Module, welche von anderen genutzt werden, werden in einem weiteren branch entwickelt, sodass das Modul auch während der entwicklung zur Verfügung steht.

########################################
Planed features:
-I2c
-SPI
-Math-Eigen-lib
-Sound
--Output
--Inout
-Keyboard ShortcutHandler
-Inet
--Socket-connection
--POST + GET
-Usb als serielle Schnittstelle
-Parser
--Json
--XML
