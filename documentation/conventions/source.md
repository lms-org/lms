####Source:
 * Wer hat es erstellt
 * Wann wurde es erstellt
 * was soll es tun
 * Code-Style should be the same in the framework.
 * files are named using lower letters/snake-case

####Header:
For jede Methode:
 * Falls der Methodennamen nicht selbsterklärend ist (Nicht für einen selbst, sondern für jemand anderen, einen kurzen Kommentar)
 * header-guard: projectname_classname_h


####Cpp/Implementation
 * Bei jedem Hack wird ein Kommentar mit der Formatierung:
    //TODO: HACK {Was er tut}

 * Falls die Datei unvollständig ist über die includes einen TODO-Kommentar schreiben

 * Referenzen verwenden, Pointer nur wenn nötig.
 * See [cppguide](google-styleguide.googlecode.com/svn/trunk/cppguide.html)
 * See also [cpp-files](www.possibility.com/Cpp/CppCodingStandard.html)
