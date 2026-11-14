"""
Writes the game's text as CSVs that Unreal imports as DataTables.
Edit the lines here, run `python make_text_tables.py`, re-import the CSVs.

Placeholders filled in at runtime:
  {EMP}        player's employee number (4471)
  {TAKE}       current take number
  {GLYPHS_A}   {GLYPHS_B}  {GLYPHS_C}   pieces of the current Key Lock target, drawn as glyphs
Company name is a placeholder: HALCYON.
"""
import csv
import os

HERE = os.path.dirname(os.path.abspath(__file__))


def write(name, header, rows):
    with open(os.path.join(HERE, name), "w", newline="", encoding="utf-8") as f:
        w = csv.writer(f, quoting=csv.QUOTE_ALL)
        w.writerow(header)
        w.writerows(rows)
    print(f"{name}: {len(rows)} rows")


# ---------------------------------------------------------------------------
# EMAILS  (struct S_Email: Act int, Sender str, Subject str, Body str,
#          HasGlyphClue bool, IsPhishing bool, ArrivesAfter str)
# ---------------------------------------------------------------------------
EMAILS = [
    # --- Act 1: Morning ---
    ("A1_Welcome", 1, "HALCYON People Team", "Welcome back, {EMP}!",
     "Good morning, {EMP}!\n\nWe hope you slept well. Today is a big day, and we're so glad you're here for it.\n\n"
     "Your task list is on your badge. Please complete Mandatory Compliance Training before noon.\n\n"
     "Remember: you are exactly where you need to be.\n\nWarmly,\nThe People Team",
     False, False, "Start"),
    ("A1_ChairmanMemo", 1, "The Chairman", "Places, everyone.",
     "Team,\n\nBig day. Everyone knows their marks.\n\nLet's keep it natural. Let's keep it moving. "
     "And let's get it in one take this time.\n\n- C",
     False, False, "Start"),
    ("A1_ITNotice", 1, "IT Service Desk", "Scheduled maintenance: Corridor B",
     "Hello,\n\nCorridor B will undergo routine continuity maintenance today. You may notice flickering lights, "
     "repeated signage, or a brief sense that you have walked this way before. This is expected.\n\n"
     "Please do not report it.\n\nIT Service Desk\n\n{GLYPHS_A}",
     True, False, "Start"),
    ("A1_Coffee", 1, "Break Room Bot", "Fresh coffee is ready ☕",
     "A fresh pot has been brewed for you, {EMP}.\nIt is exactly the temperature you like.\nIt has always been exactly the temperature you like.",
     False, False, "AfterEmails"),

    # --- Act 2: Midday ---
    ("A2_Lunch", 2, "HALCYON People Team", "Lunch is served",
     "Lunch is served.\n\nLunch is served.\n\nPlease enjoy your lunch. Lunch is served.",
     False, False, "Act2Start"),
    ("A2_CallInvite", 2, "Calendar", "Invite: Sync (12:30)",
     "Sync @ 12:30, Conference Room\nOrganizer: C\nCamera: optional (organizer's camera will be off)\n\n"
     "Agenda:\n1. Welcome\n2. Quarterly energy targets\n3. Morale\n4. Retake",
     False, False, "Act2Start"),
    ("A2_PayrollPhish", 2, "Payro11 Department", "ACTION REQUIRED: verify your key",
     "Dear Employee,\n\nTo keep receiving your energy allocation, reply with your key signature below. "
     "This is the correct one. Trust this one.\n\n{GLYPHS_B}",
     True, True, "AfterCall"),
    ("A2_Facilities", 2, "Facilities", "Re: the clocks",
     "We are aware that all clocks on the floor read 2:14.\nIt is 2:14.\n\n- Facilities\n\n{GLYPHS_B}",
     True, False, "AfterMeeting"),
    ("A2_ChairmanMemo", 2, "The Chairman", "Notes from this morning",
     "{EMP},\n\nGood energy this morning. A little hesitant at the corridor. We'll go again.\n\n"
     "Don't look at the camera.\n\n- C",
     False, False, "AfterMeeting"),

    # --- Act 3: Overtime ---
    ("A3_TeamBuilding", 3, "HALCYON People Team", "Mandatory team building: Wellness Center",
     "Overtime is a privilege!\n\nJoin us in the Wellness Center for a short trust exercise. "
     "Your team will be there to catch you.\n\nAttendance is mandatory. Attendance has always been mandatory.",
     False, False, "Act3Start"),
    ("A3_ChairmanMemo", 3, "The Chairman", "That's nearly a wrap",
     "{EMP},\n\nYou've been wonderful. One more scene.\n\nThe button you've been looking at will light up soon.\n\n- C",
     False, False, "AfterKey3"),
]

# ---------------------------------------------------------------------------
# PA ANNOUNCEMENTS  (struct S_PALine: Act int, Trigger str, Line str, Repeats int)
# ---------------------------------------------------------------------------
PA = [
    ("PA_A1_Open", 1, "GameStart", "Good morning, team. The Chairman thanks you for your energy.", 1),
    ("PA_A1_Training", 1, "AfterCoffee", "Reminder: Mandatory Compliance Training closes at noon. Compliance is a kindness.", 1),
    ("PA_A1_CorridorB", 1, "EnterCorridorB", "Corridor B is perfectly safe. Please continue walking.", 1),
    ("PA_A1_Rewind", 1, "AfterCapture", "Thank you for your patience. We are going again from the top.", 1),
    ("PA_A2_Lunch", 2, "Act2Start", "Lunch is served.", 3),
    ("PA_A2_Clocks", 2, "ClocksFreeze", "The time is two fourteen. The time is two fourteen.", 1),
    ("PA_A2_Mannequins", 2, "MannequinsActive", "Please remain in your positions.", 1),
    ("PA_A3_Overtime", 3, "Act3Start", "Overtime has begun. Overtime is a privilege.", 1),
    ("PA_A3_Hospital", 3, "EnterHospital", "Wellness is mandatory. Please lie still.", 1),
    ("PA_END_Wrap", 4, "EndingStart", "That's a wrap on Employee {EMP}! That's a wrap on Employee {EMP}!", 2),
    ("PA_PERFORMANCE", 0, "Random", "The Chairman thanks you for your performance.", 1),
]

# ---------------------------------------------------------------------------
# COMPLIANCE TRAINING: Data Integrity Module 1 (Key 1 tutorial)
# (struct S_TrainingSlide: Order int, Title str, Body str, Kind str [Slide|Quiz|KeyLock], Options str)
# ---------------------------------------------------------------------------
TRAINING = [
    ("T01", 1, "Data Integrity Module 1", "Welcome, {EMP}! This short module keeps our data, and you, intact.\n\nEstimated time: 3 minutes.\nEstimated time: forever.", "Slide", ""),
    ("T02", 2, "What is a Signature?", "Every HALCYON key has a SIGNATURE: a row of glyphs, each one LIT or UNLIT.", "Slide", ""),
    ("T03", 3, "Fragments", "Keys are assembled from FRAGMENTS. Each fragment is also a row of glyphs.\n\nYou may switch fragments ON or OFF.", "Slide", ""),
    ("T04", 4, "The Integrity Rule", "Look down each column of glyphs.\n\nIf an ODD number of your active fragments are lit there, the result is LIT.\nIf an EVEN number are lit (or none), the result is UNLIT.", "Slide", ""),
    ("T05", 5, "Practice", "Combine fragments until your RESULT matches the SIGNATURE.\nThe result is shown live for this module only.", "KeyLock", ""),
    ("T06", 6, "Well Done!", "Integrity confirmed. Future keys will not show you the signature.\nYou will have to find it.\n\nIt is written all around you.", "Slide", ""),
    ("T07", 7, "Quick Check", "Malware may disguise itself as a fragment. What should you do?", "Quiz", "Trust every fragment|Report it to your manager|Look closer"),
    ("T08", 8, "Final Question", "Do you consent to your next take?", "Quiz", "Yes|Yes"),
]

# ---------------------------------------------------------------------------
# NPC LINES  (struct S_NPCLine: Role str, Act int, IsAgent bool, Context str, Line str)
# Agents sound just as friendly; they hand out false clues.
# ---------------------------------------------------------------------------
NPC = [
    ("C_A1_Cooler1", "Coworker", 1, False, "Greeting", "Morning! You look rested. You always look rested."),
    ("C_A1_Cooler2", "Coworker", 1, False, "Greeting", "Did you take the long way past Corridor B? Don't take the long way."),
    ("C_A2_Clue1", "Coworker", 2, False, "Clue", "IT keeps sending weird symbols at the bottom of their emails. I wrote one down: {GLYPHS_A}"),
    ("C_A2_Clue2", "Coworker", 2, False, "Clue", "Did you see the whiteboard after the meeting? Someone drew those little marks again."),
    ("C_A2_Agent1", "Coworker", 2, True, "FalseClue", "Oh, the payroll email? That one's legit. Copy it exactly. {GLYPHS_B}"),
    ("C_A2_Agent2", "Coworker", 2, True, "FalseClue", "The closet code? Easy. Just use every fragment. All of them. Trust me."),
    ("C_A2_Mannequin", "Coworker", 2, False, "Ambient", "..."),
    ("G_A1_Check", "Guard", 1, False, "BadgeCheck", "Badge, please. ...Thank you, {EMP}. You're not cleared for this door. Yet."),
    ("G_A2_Cleared", "Guard", 2, False, "PassOK", "Cleared. Go on through. Don't run in the halls."),
    ("G_A2_Agent", "Guard", 2, True, "PassLie", "That pass has been revoked. Hand it over and I'll get you a new one."),
    ("G_Fear_Radio", "Guard", 0, True, "FearRadio", "(into radio) {EMP}'s on the floor. Yeah. Same as last take."),
    ("J_A1_Mop", "Custodian", 1, False, "Greeting", "Mind the floor, it's wet. It's always wet over there."),
    ("J_A2_Hint", "Custodian", 2, False, "Hint", "They only clean the posters in Corridor B. Funny. One of them's always new."),
    ("J_A3_Hint", "Custodian", 3, False, "Hint", "Nobody reads the charts in Wellness. I do. They're about you, you know."),
    # --- Act 1 bump / small talk (random line when you walk into someone or press E) ---
    ("C_A1_Bump1", "Coworker", 1, False, "Bump", "Oh! Sorry. Didn't see you there. Nobody ever sees you there."),
    ("C_A1_Bump2", "Coworker", 1, False, "Bump", "Coffee machine's making that noise again. The happy noise."),
    ("C_A1_Bump3", "Coworker", 1, False, "Bump", "Did you finish your compliance training? It's mandatory. Everything is."),
    ("C_A1_Bump4", "Coworker", 1, False, "Bump", "Inbox zero! Just kidding. There's always one more."),
    ("C_A1_Bump5", "Coworker", 1, False, "Bump", "Love the energy today, {EMP}. Same as yesterday. Love that too."),
    ("C_A1_Bump6", "Coworker", 1, False, "Bump", "Printer's jammed. It's fine. I'll stand here until it isn't."),
    ("C_A1_Bump7", "Coworker", 1, False, "Bump", "Is it still morning? Feels like it's been morning for a while."),
    ("C_A1_Bump8", "Coworker", 1, False, "Bump", "Big day. Everyone knows their marks."),
    ("C_A1_Bump9", "Coworker", 1, False, "Bump", "Someone took my yogurt. Again. Read the note."),
    ("C_A1_Bump10", "Coworker", 1, False, "Bump", "Can't talk, quarterly synergy deck. Due yesterday. Always due yesterday."),
    ("C_A1_Bump11", "Coworker", 1, False, "Bump", "Have you met the Chairman? Me neither. He's met us, though."),
    ("C_A1_Bump12", "Coworker", 1, False, "Bump", "Mind the planter. It moved. Plants don't move, right?"),
    ("C_A1_Agent1", "Coworker", 1, True, "Bump", "Your desk is the one by the window. Was it always by the window?"),
    ("C_A1_Agent2", "Coworker", 1, True, "Bump", "Smile, {EMP}. Camera's always rolling."),
    ("G_A1_Bump1", "Guard", 1, False, "Bump", "Keep it moving."),
    ("G_A1_Bump2", "Guard", 1, False, "Bump", "Corridor B is closed for maintenance. Permanently. Temporarily."),
    ("G_A1_Bump3", "Guard", 1, False, "Bump", "Lanyard visible at all times, please."),
    ("J_A1_Bump1", "Custodian", 1, False, "Bump", "Same scuffs every morning. Same exact scuffs."),
    ("J_A1_Bump2", "Custodian", 1, False, "Bump", "Don't go in my closet. Nothing in there. Mops."),
    ("J_A1_Bump3", "Custodian", 1, False, "Bump", "Whoa there. Wet floor."),
    # --- more Act 2 ---
    ("C_A2_Call1", "Coworker", 2, False, "VideoCall", "Can everyone see my screen? ...No? It's just you on it. Weird."),
    ("C_A2_Call2", "Coworker", 2, False, "VideoCall", "C is on the call. C is always on the call."),
    ("C_A2_Meeting", "Coworker", 2, False, "Meeting", "Item four is you. It's always been you. Don't take it personally."),
    ("C_A2_Clocks", "Coworker", 2, False, "Ambient", "Has it been two fourteen for a while? It feels like it's been two fourteen for a while."),
    ("C_A2_Agent3", "Coworker", 2, True, "FalseClue", "The whiteboard's wrong, by the way. They redraw it. Use the one in the payroll email."),
    ("C_A2_Retake", "Coworker", 2, False, "AfterRewind", "Oh, you're back! I mean... good morning. Again."),
    ("G_A2_Closet", "Guard", 2, False, "BadgeCheck", "Supply closet? Pass one gets you in. Pass two gets you further. Nobody's ever needed pass three."),
    ("J_A2_Cart", "Custodian", 2, False, "Hint", "If it chases you, put something in its way. It hates a mess. I'd know."),
    # --- Act 3 ---
    ("C_A3_Circle", "Coworker", 3, False, "TeamBuilding", "Arms out, eyes closed. We've got you. We've always got you."),
    ("C_A3_Circle2", "Coworker", 3, True, "TeamBuilding", "Lean back. Further. Further than that."),
    ("N_A3_Nurse", "Coworker", 3, True, "Hospital", "You're awake early. That isn't in the script. Lie back down."),
    ("J_A3_Ward", "Custodian", 3, False, "Hint", "Charts with a coffee ring are the real ones. Props never drink coffee."),
    # --- Wrap party ---
    ("P_Coworker", "Coworker", 4, False, "Party", "You were incredible. Honestly. The corridor bit? Chills."),
    ("P_Guard", "Guard", 4, False, "Party", "No hard feelings about the radio thing. Just doing my blocking."),
    ("P_Custodian", "Custodian", 4, False, "Party", "Good work. Don't hand them anything."),
    ("P_Stalker", "Stalker", 4, False, "Party", "(mask under one arm, coffee in the other) Sorry about the chasing. You're really fast."),
]

# ---------------------------------------------------------------------------
# SCRIPT PAGES (Act 3 patient charts; struct S_ScriptPage: Act int, Scene str, Text str, HasGlyphClue bool, IsFake bool)
# ---------------------------------------------------------------------------
SCRIPT = [
    ("S_01", 3, "INT. WELLNESS CENTER - OVERTIME",
     "EMPLOYEE {EMP} wakes on the floor. They do not remember falling. They look at this page.", False, False),
    ("S_02", 3, "INT. WARD - CONTINUOUS",
     "{EMP} reads the chart. {EMP} realises the chart is reading back.\n\n{GLYPHS_C}", True, False),
    ("S_03", 3, "INT. WARD - CONTINUOUS",
     "{EMP} trusts this page. This page is correct. (Note from props: replace before take {TAKE}.)\n\n{GLYPHS_B}", True, True),
    ("S_04", 3, "INT. ELEVATOR - LATER",
     "The button marked C lights. {EMP} hesitates. Hold on the hesitation.", False, False),
]


# ---------------------------------------------------------------------------
# PROP TEXT: readable props in the world (struct S_PropText: Where str, Kind str, Text str)
# ---------------------------------------------------------------------------
PROPS = [
    ("CallSheet", "BulletinBoard", "CallSheet",
     "HALCYON CONTINUITY: CALL SHEET\nDay 1 of 1 · Take {TAKE}\nDirector: C\n\n"
     "CAST\n  EMPLOYEE {EMP} ........ Themself\n  COWORKERS ........... As cast\n  SECURITY ............ As cast\n  THE DARK FIGURE ..... Stunt\n\n"
     "SCHEDULE\n  09:00 Desk / Email\n  10:00 Corridor B (night look)\n  12:00 Lunch (repeat as needed)\n"
     "  14:14 Hold\n  16:00 Wellness (trust fall: NO MATS)\n  ??:?? Wrap\n\nNOTES\n  Do not let {EMP} see this sheet."),
    ("Whiteboard", "ConferenceRoom", "Whiteboard",
     "Q4 ENERGY TARGETS ↑↑↑\nRetention: 100%\nMorale: mandatory\n\n{GLYPHS_B}\n\n(do not erase)"),
    ("Agenda", "CallScreen", "Agenda",
     "SYNC · 12:30\n1. Welcome\n2. Quarterly energy targets\n3. Morale\n4. Retake ({EMP})"),
    ("Plaque", "Lobby", "Portrait", "THE CHAIRMAN\nFounder · Director · Friend"),
    ("ElevatorPanel", "Elevator", "Sign", "1   2   3   C\n(C: authorised personnel only)"),
    ("WetFloor", "Anywhere", "Sign", "CAUTION\nWET FLOOR\n(always)"),
    ("CorridorBSign", "CorridorB", "Sign", "CORRIDOR B\nYou have always been walking here."),
    ("WellnessSign", "WellnessCenter", "Sign", "WELLNESS CENTER\nWellness is mandatory."),
]

# ---------------------------------------------------------------------------
# ENDING SEQUENCE (struct S_EndingLine: Order int, Beat str, Speaker str, Line str, Branch str [All|Yes|No])
# ---------------------------------------------------------------------------
ENDING = [
    ("E01", 1, "Doors", "", "(The elevator doors open. Light. Confetti. Applause.)", "All"),
    ("E02", 2, "Wrap", "PA", "That's a wrap on Employee {EMP}! That's a wrap on Employee {EMP}!", "All"),
    ("E03", 3, "Wrap", "Coworker", "You were incredible. We're so proud of you.", "All"),
    ("E04", 4, "Off", "", "(The clock on the wall still reads 2:14.)", "All"),
    ("E05", 5, "Collector", "The Collector", "Congratulations, {EMP}. Really. Not many make it to the end of the take.", "All"),
    ("E06", 6, "Collector", "The Collector", "The Chairman watched every second. He thinks you've earned your director's cut.", "All"),
    ("E07", 7, "Collector", "The Collector", "The astral realm is open now. You can build yourself any world you want. A palace. An afterlife. No more corridors.", "All"),
    ("E08", 8, "Collector", "The Collector", "You just have to hand over your keys.", "All"),
    ("E09", 9, "Choice", "", "HAND OVER YOUR KEYS?   [ YES ]   [ NO ]", "All"),
    ("E10", 10, "Yes", "The Collector", "Wonderful. Your world is being built as we speak. You'll love it. Everyone does.", "Yes"),
    ("E11", 11, "Yes", "", "(A bright palace. Perfect lawns. A warm sun that never moves. Everyone you meet is smiling, a beat too long.)", "Yes"),
    ("E12", 12, "Yes", "", "THE THEATRICAL CUT", "Yes"),
    ("E13", 13, "No", "The Collector", "Of course. Of course. Take your time.", "No"),
    ("E14", 14, "No", "", "(The passes are no longer on your badge. You never felt them go.)", "No"),
    ("E15", 15, "No", "", "(For three seconds: the uncut footage. The real office. Empty desks. A long line of badges on hooks, each with a number.)", "No"),
    ("E16", 16, "No", "", "THE DIRECTOR'S CUT", "No"),
    ("E17", 17, "Final", "", "(Soul Sight flickers on the Collector. Code starts to resolve. Cut to black.)", "All"),
]

# ---------------------------------------------------------------------------
# CREDITS (struct S_CreditLine: Order int, Role str, Name str)
# ---------------------------------------------------------------------------
CREDITS = [
    ("C01", 1, "TAKE 4471", ""),
    ("C02", 2, "A HALCYON Continuity production", ""),
    ("C03", 3, "Game design, art, music", "phoenixlynx (SkyYamaha)"),
    ("C04", 4, "Development partner", "Claude (Anthropic)"),
    ("C05", 5, "Made in", "Unreal Engine 5.8 for the 2026 Epic AI MegaJam"),
    ("C06", 6, "Theme", "Director's Cut"),
    ("C07", 7, "Employee {EMP}", "Themself"),
    ("C08", 8, "Takes", "{TAKE}"),
    ("C09", 9, "Directed by", ""),
]


if __name__ == "__main__":
    write("DT_Emails.csv", ["Name", "Act", "Sender", "Subject", "Body", "HasGlyphClue", "IsPhishing", "ArrivesAfter"], EMAILS)
    write("DT_PALines.csv", ["Name", "Act", "Trigger", "Line", "Repeats"], PA)
    write("DT_ComplianceTraining.csv", ["Name", "Order", "Title", "Body", "Kind", "Options"], TRAINING)
    write("DT_NPCLines.csv", ["Name", "Role", "Act", "IsAgent", "Context", "Line"], NPC)
    write("DT_ScriptPages.csv", ["Name", "Act", "Scene", "Text", "HasGlyphClue", "IsFake"], SCRIPT)
    write("DT_PropText.csv", ["Name", "Where", "Kind", "Text"], PROPS)
    write("DT_Ending.csv", ["Name", "Order", "Beat", "Speaker", "Line", "Branch"], ENDING)
    write("DT_Credits.csv", ["Name", "Order", "Role", "CreditName"], CREDITS)
