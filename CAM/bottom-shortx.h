0 BEGIN PGM bottom-shortx MM
1 BLK FORM 0.1 Z X+0 Y+0 Z-12
2 BLK FORM 0.2 X+326 Y+400 Z+0
3 ;-------------------------------------
4 ;Werkzeuge
5 ;  #15 D=4.5 KONIK=118(Grad): - ZMIN=-14.352 - ZMAX=+160 - Bohrer
6 ;-------------------------------------
7 ;
8 * - Drill
9 M5
10 TOOL CALL 15 Z S4000
11 L M140 MB MAX
12 M3
13 L X+10 Y+41.5 R0 FMAX
14 L Z+160 R0 FMAX
15 CYCL DEF 32.0 TOLERANZ
16 CYCL DEF 32.1
17 CYCL DEF 200 BOHREN ~
  Q200=+5 ;SICHERHEITS-ABST. ~
  Q201=-14.352 ;TIEFE ~
  Q206=700 ;VORSCHUB TIEFENZ. ~
  Q202=+14.352 ;ZUSTELL-TIEFE ~
  Q210=0 ;VERWEILZEIT OBEN ~
  Q203=+0 ;KOOR. OBERFLAECHE ~
  Q204=+5 ;2. SICHERHEITS-ABST. ~
  Q211=0 ;VERWEILZEIT UNTEN
18 L FMAX M99
19 L X+29.5 Y+22 FMAX M99
20 L X+102.1 Y+41.5 FMAX M99
21 L X+223.9 FMAX M99
22 L X+296.5 Y+22 FMAX M99
23 L X+316 Y+41.5 FMAX M99
24 L X+261.7 Y+107 FMAX M99
25 L X+64.3 FMAX M99
26 L X+163 Y+143.5 FMAX M99
27 CYCL DEF 200 BOHREN ~
  Q200=+5 ;SICHERHEITS-ABST. ~
  Q201=-7.352 ;TIEFE ~
  Q206=700 ;VORSCHUB TIEFENZ. ~
  Q202=+7.352 ;ZUSTELL-TIEFE ~
  Q210=0 ;VERWEILZEIT OBEN ~
  Q203=+0 ;KOOR. OBERFLAECHE ~
  Q204=+5 ;2. SICHERHEITS-ABST. ~
  Q211=0 ;VERWEILZEIT UNTEN
28 L X+177.415 Y+193.81 FMAX M99
29 L X+219.706 Y+232.545 FMAX M99
30 L X+106.168 Y+233.434 FMAX M99
31 L X+106.549 Y+286.139 FMAX M99
32 L X+152.904 Y+301.252 FMAX M99
33 L X+217.801 Y+309.38 FMAX M99
34 CYCL DEF 200 BOHREN ~
  Q200=+5 ;SICHERHEITS-ABST. ~
  Q201=-14.352 ;TIEFE ~
  Q206=700 ;VORSCHUB TIEFENZ. ~
  Q202=+14.352 ;ZUSTELL-TIEFE ~
  Q210=0 ;VERWEILZEIT OBEN ~
  Q203=+0 ;KOOR. OBERFLAECHE ~
  Q204=+5 ;2. SICHERHEITS-ABST. ~
  Q211=0 ;VERWEILZEIT UNTEN
35 L X+316 Y+358.5 FMAX M99
36 L X+296.5 Y+378 FMAX M99
37 L X+29.5 FMAX M99
38 L X+10 Y+358.5 FMAX M99
39 L Z+160 FMAX
40 M5
41 L M140 MB MAX
42 M30
43 END PGM bottom-shortx MM
