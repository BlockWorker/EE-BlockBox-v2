0 BEGIN PGM bottom-longx MM
1 BLK FORM 0.1 Z X+0 Y+0 Z-12
2 BLK FORM 0.2 X+400 Y+326 Z+0
3 ;-------------------------------------
4 ;Werkzeuge
5 ;  #15 D=4.5 KONIK=118(Grad): - ZMIN=-14.352 - ZMAX=+160 - Bohrer
6 ;-------------------------------------
7 ;
8 * - Drill 1
9 M5
10 TOOL CALL 15 Z S4000
11 L M140 MB MAX
12 M3
13 L X+41.5 Y+10 R0 FMAX
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
19 L X+22 Y+29.5 FMAX M99
20 L X+41.5 Y+102.1 FMAX M99
21 L X+107 Y+64.3 FMAX M99
22 L X+358.5 Y+10 FMAX M99
23 L X+378 Y+29.5 FMAX M99
24 L Z+160 FMAX
25 * - Drill 2
26 M3
27 L X+309.38 Y+108.199 R0 FMAX
28 L Z+160 R0 FMAX
29 CYCL DEF 200 BOHREN ~
  Q200=+5 ;SICHERHEITS-ABST. ~
  Q201=-7.352 ;TIEFE ~
  Q206=700 ;VORSCHUB TIEFENZ. ~
  Q202=+7.352 ;ZUSTELL-TIEFE ~
  Q210=0 ;VERWEILZEIT OBEN ~
  Q203=+0 ;KOOR. OBERFLAECHE ~
  Q204=+5 ;2. SICHERHEITS-ABST. ~
  Q211=0 ;VERWEILZEIT UNTEN
30 L FMAX M99
31 L X+232.545 Y+106.294 FMAX M99
32 L X+193.81 Y+148.585 FMAX M99
33 CYCL DEF 200 BOHREN ~
  Q200=+5 ;SICHERHEITS-ABST. ~
  Q201=-14.352 ;TIEFE ~
  Q206=700 ;VORSCHUB TIEFENZ. ~
  Q202=+14.352 ;ZUSTELL-TIEFE ~
  Q210=0 ;VERWEILZEIT OBEN ~
  Q203=+0 ;KOOR. OBERFLAECHE ~
  Q204=+5 ;2. SICHERHEITS-ABST. ~
  Q211=0 ;VERWEILZEIT UNTEN
34 L X+143.5 Y+163 FMAX M99
35 CYCL DEF 200 BOHREN ~
  Q200=+5 ;SICHERHEITS-ABST. ~
  Q201=-7.352 ;TIEFE ~
  Q206=700 ;VORSCHUB TIEFENZ. ~
  Q202=+7.352 ;ZUSTELL-TIEFE ~
  Q210=0 ;VERWEILZEIT OBEN ~
  Q203=+0 ;KOOR. OBERFLAECHE ~
  Q204=+5 ;2. SICHERHEITS-ABST. ~
  Q211=0 ;VERWEILZEIT UNTEN
36 L X+301.252 Y+173.096 FMAX M99
37 L X+233.434 Y+219.832 FMAX M99
38 L X+286.139 Y+219.451 FMAX M99
39 L Z+160 FMAX
40 * - Drill 3
41 M3
42 L X+378 Y+296.5 R0 FMAX
43 L Z+160 R0 FMAX
44 CYCL DEF 200 BOHREN ~
  Q200=+5 ;SICHERHEITS-ABST. ~
  Q201=-14.352 ;TIEFE ~
  Q206=700 ;VORSCHUB TIEFENZ. ~
  Q202=+14.352 ;ZUSTELL-TIEFE ~
  Q210=0 ;VERWEILZEIT OBEN ~
  Q203=+0 ;KOOR. OBERFLAECHE ~
  Q204=+5 ;2. SICHERHEITS-ABST. ~
  Q211=0 ;VERWEILZEIT UNTEN
45 L FMAX M99
46 L X+358.5 Y+316 FMAX M99
47 L X+107 Y+261.7 FMAX M99
48 L X+41.5 Y+223.9 FMAX M99
49 L X+22 Y+296.5 FMAX M99
50 L X+41.5 Y+316 FMAX M99
51 L Z+160 FMAX
52 M5
53 L M140 MB MAX
54 M30
55 END PGM bottom-longx MM