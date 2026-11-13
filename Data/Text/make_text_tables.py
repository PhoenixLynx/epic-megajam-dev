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


if __name__ == "__main__":
    write("DT_Emails.csv", ["Name", "Act", "Sender", "Subject", "Body", "HasGlyphClue", "IsPhishing", "ArrivesAfter"], EMAILS)
    write("DT_PALines.csv", ["Name", "Act", "Trigger", "Line", "Repeats"], PA)
    write("DT_ComplianceTraining.csv", ["Name", "Order", "Title", "Body", "Kind", "Options"], TRAINING)
    write("DT_NPCLines.csv", ["Name", "Role", "Act", "IsAgent", "Context", "Line"], NPC)
    write("DT_ScriptPages.csv", ["Name", "Act", "Scene", "Text", "HasGlyphClue", "IsFake"], SCRIPT)
