import os
import subprocess
from PIL import Image, ImageDraw, ImageFont

# Ensure screenshots directory exists
os.makedirs("screenshots", exist_ok=True)

REG_FONT_PATH = "/usr/share/fonts/TTF/JetBrainsMonoNerdFont-Regular.ttf"
BOLD_FONT_PATH = "/usr/share/fonts/TTF/JetBrainsMonoNerdFont-Bold.ttf"

FONT_SIZE = 14
LINE_HEIGHT = 22
PAD_X = 24
PAD_Y = 18
TITLE_BAR_HEIGHT = 38

# Color Palette (Catppuccin Mocha themed)
BG_COLOR = (24, 24, 37)          # #181825 Terminal Body
BAR_COLOR = (30, 30, 46)         # #1e1e2e Title Bar
BORDER_COLOR = (49, 50, 68)      # #313244
SHADOW_COLOR = (17, 17, 27)      # #11111b
PROMPT_USER = (166, 227, 161)    # #a6e3a1 Green
PROMPT_DIR = (137, 180, 250)     # #89b4fa Blue
CMD_COLOR = (245, 224, 220)      # #f5e0dc Light Rose
TEXT_WHITE = (205, 214, 244)     # #cdd6f4
TEXT_GRAY = (147, 153, 178)      # #9399b2
TEXT_CYAN = (137, 220, 235)      # #89dceb Cyan
TEXT_GREEN = (166, 227, 161)     # #a6e3a1 Green
TEXT_RED = (243, 139, 168)       # #f38ba8 Red
TEXT_YELLOW = (249, 226, 175)    # #f9e2af Yellow
TEXT_PURPLE = (203, 166, 247)    # #cba6f7 Mauve
TEXT_ORANGE = (250, 179, 135)    # #fab387 Peach
TEXT_INPUT = (180, 190, 254)     # #b4befe Lavender

FONT_REG = ImageFont.truetype(REG_FONT_PATH, FONT_SIZE)
FONT_BOLD = ImageFont.truetype(BOLD_FONT_PATH, FONT_SIZE)
FONT_TITLE = ImageFont.truetype(BOLD_FONT_PATH, 12)

def style_line(line, is_input=False):
    """Returns segments [(text, color, is_bold), ...]"""
    if is_input:
        return [(line, TEXT_INPUT, False)]
    
    line_str = line.strip()
    if line_str.startswith("Algorithm:"):
        parts = line.split(":", 1)
        return [(parts[0] + ":", TEXT_CYAN, True), (parts[1], TEXT_PURPLE, True)]
    elif line_str.startswith("Problem:"):
        parts = line.split(":", 1)
        return [(parts[0] + ":", TEXT_CYAN, True), (parts[1], TEXT_WHITE, False)]
    elif line_str.startswith("Path Found:"):
        parts = line.split(":", 1)
        val = parts[1].strip()
        color = TEXT_GREEN if val == "Yes" else TEXT_RED
        return [(parts[0] + ":", TEXT_WHITE, False), (" " + val, color, True)]
    elif line_str.startswith("Best Move:"):
        parts = line.split(":", 1)
        return [(parts[0] + ":", TEXT_WHITE, False), (parts[1], TEXT_GREEN, True)]
    elif line_str.startswith("Total Cost") or line_str.startswith("Final Cost"):
        parts = line.split("=", 1) if "=" in line else line.split(":", 1)
        sep = "=" if "=" in line else ":"
        return [(parts[0] + sep, TEXT_YELLOW, True), (parts[1], TEXT_YELLOW, True)]
    elif line_str.startswith("Nodes Pruned"):
        parts = line.split("=", 1)
        return [(parts[0] + "=", TEXT_WHITE, False), (parts[1], TEXT_RED, True)]
    elif "Execution Time =" in line:
        parts = line.split("=", 1)
        return [(parts[0] + "=", TEXT_GRAY, False), (parts[1], TEXT_PURPLE, False)]
    elif line_str.startswith("===") or line_str.startswith("---"):
        return [(line, TEXT_GRAY, False)]
    elif "→ Slot" in line:
        parts = line.split("→")
        return [(parts[0], TEXT_ORANGE, True), ("→" + parts[1], TEXT_WHITE, False)]
    elif line_str.startswith("Iteration") or line_str.startswith("Initial Timetable:") or line_str.startswith("Final Timetable:"):
        return [(line, TEXT_CYAN, True)]
    elif "Cost =" in line:
        parts = line.split("=")
        return [(parts[0] + "=", TEXT_YELLOW, False), (parts[1], TEXT_YELLOW, True)]
    elif line_str.startswith("Path:"):
        parts = line.split(":", 1)
        return [(parts[0] + ":", TEXT_WHITE, False), (parts[1], TEXT_ORANGE, True)]
    elif "=" in line:
        parts = line.split("=", 1)
        return [(parts[0] + "=", TEXT_WHITE, False), (parts[1], TEXT_GREEN, False)]
    else:
        return [(line, TEXT_WHITE, False)]

def render_terminal(title, prompt_cmd, input_text, output_text, output_filename, width=920):
    # Collect lines
    rendered_rows = []
    
    # Prompt row
    rendered_rows.append({
        "type": "prompt",
        "cmd": prompt_cmd
    })
    
    # Input lines
    if input_text and input_text.strip():
        for l in input_text.strip().split("\n"):
            rendered_rows.append({
                "type": "input",
                "segments": style_line(l, is_input=True)
            })
            
    # Output lines
    for l in output_text.strip().split("\n"):
        rendered_rows.append({
            "type": "output",
            "segments": style_line(l, is_input=False)
        })
        
    height = TITLE_BAR_HEIGHT + PAD_Y * 2 + len(rendered_rows) * LINE_HEIGHT + 10
    
    img = Image.new("RGB", (width, height), color=BG_COLOR)
    draw = ImageDraw.Draw(img)
    
    # Title bar
    draw.rectangle([(0, 0), (width, TITLE_BAR_HEIGHT)], fill=BAR_COLOR)
    draw.line([(0, TITLE_BAR_HEIGHT), (width, TITLE_BAR_HEIGHT)], fill=BORDER_COLOR, width=1)
    
    # Traffic dots
    dot_y = TITLE_BAR_HEIGHT // 2
    draw.ellipse([(16, dot_y - 6), (28, dot_y + 6)], fill=(255, 95, 86))    # Close
    draw.ellipse([(36, dot_y - 6), (48, dot_y + 6)], fill=(255, 189, 46))   # Min
    draw.ellipse([(56, dot_y - 6), (68, dot_y + 6)], fill=(39, 201, 63))    # Max
    
    # Window Title
    title_box = draw.textbbox((0, 0), title, font=FONT_TITLE)
    title_w = title_box[2] - title_box[0]
    draw.text(((width - title_w) // 2, dot_y - 7), title, fill=TEXT_GRAY, font=FONT_TITLE)
    
    # Render lines
    y = TITLE_BAR_HEIGHT + PAD_Y
    
    for row in rendered_rows:
        if row["type"] == "prompt":
            x = PAD_X
            user_part = "harshal@ai-lab:"
            draw.text((x, y), user_part, fill=PROMPT_USER, font=FONT_BOLD)
            x += draw.textbbox((0, 0), user_part, font=FONT_BOLD)[2]
            
            dir_part = "~/course/ai/Group_assignement$ "
            draw.text((x, y), dir_part, fill=PROMPT_DIR, font=FONT_BOLD)
            x += draw.textbbox((0, 0), dir_part, font=FONT_BOLD)[2]
            
            cmd_part = row["cmd"]
            draw.text((x, y), cmd_part, fill=CMD_COLOR, font=FONT_BOLD)
        else:
            x = PAD_X
            if row["type"] == "input":
                # subtle stdin marker
                marker = "│ "
                draw.text((x, y), marker, fill=(88, 91, 112), font=FONT_REG)
                x += draw.textbbox((0, 0), marker, font=FONT_REG)[2]
                
            for text, color, is_bold in row["segments"]:
                f = FONT_BOLD if is_bold else FONT_REG
                draw.text((x, y), text, fill=color, font=f)
                x += draw.textbbox((0, 0), text, font=f)[2]
        y += LINE_HEIGHT
        
    # Border around image
    draw.rectangle([(0, 0), (width - 1, height - 1)], outline=BORDER_COLOR, width=1)
    
    img.save(output_filename, "PNG", optimize=True)
    print(f"Generated {output_filename} ({width}x{height})")

print("Generator functions ready.")

items = [
    {
        "title": "python3 q1.py — Sample 1 (Path Exists)",
        "cmd": "python3 q1.py",
        "input": "5 5\nS..#.\n#..#.\n.....\n.##..\n....G",
        "script": "q1.py",
        "filename": "screenshots/q1_sample1.png",
        "width": 860
    },
    {
        "title": "python3 q1.py — Sample 2 (No Path / Blocked)",
        "cmd": "python3 q1.py",
        "input": "5 5\nS....\n####.\n....#\n.####\n....G",
        "script": "q1.py",
        "filename": "screenshots/q1_sample2.png",
        "width": 860
    },
    {
        "title": "python3 q2.py — Sample 1 (Campus Graph)",
        "cmd": "python3 q2.py",
        "input": "6 7\nA B 4\nA C 2\nB D 5\nC D 3\nC E 6\nD E 3\nE F 2\nA F\nA 7\nB 8\nC 5\nD 4\nE 2\nF 0",
        "script": "q2.py",
        "filename": "screenshots/q2_sample1.png",
        "width": 860
    },
    {
        "title": "python3 q2.py < p2_test_case_2.txt — Suboptimality Test Case",
        "cmd": "python3 q2.py < p2_test_case_2.txt",
        "input": "",
        "stdin_override": open("p2_test_case_2.txt").read(),
        "script": "q2.py",
        "filename": "screenshots/q2_sample2.png",
        "width": 860
    },
    {
        "title": "python3 q3.py — Sample 1 (Search Depth = 5)",
        "cmd": "python3 q3.py",
        "input": "5 7\nA.E.#..\n.#..#E.\n...#...\n#E#..#.\n....E.B\nMAX\n5",
        "script": "q3.py",
        "filename": "screenshots/q3_sample1.png",
        "width": 860
    },
    {
        "title": "python3 q3.py — Sample 2 (Search Depth = 4)",
        "cmd": "python3 q3.py",
        "input": "5 7\nA...E..\n.###...\n..E....\n...###.\n.E...B.\nMAX\n4",
        "script": "q3.py",
        "filename": "screenshots/q3_sample2.png",
        "width": 860
    },
    {
        "title": "python3 q4.py — Experiment 1 (Initial: 1 1 2 3 4 2)",
        "cmd": "python3 q4.py",
        "input": "6 4\n8\n1 2\n1 3\n2 4\n2 5\n3 4\n3 6\n4 5\n5 6\n1 1 2 3 4 2",
        "script": "q4.py",
        "filename": "screenshots/q4_exp1.png",
        "width": 860
    },
    {
        "title": "python3 q4.py — Experiment 2 (Initial: 2 3 1 1 4 4)",
        "cmd": "python3 q4.py",
        "input": "6 4\n8\n1 2\n1 3\n2 4\n2 5\n3 4\n3 6\n4 5\n5 6\n2 3 1 1 4 4",
        "script": "q4.py",
        "filename": "screenshots/q4_exp2.png",
        "width": 860
    },
    {
        "title": "python3 q4.py — Experiment 3 (Initial: 4 2 3 1 2 3)",
        "cmd": "python3 q4.py",
        "input": "6 4\n8\n1 2\n1 3\n2 4\n2 5\n3 4\n3 6\n4 5\n5 6\n4 2 3 1 2 3",
        "script": "q4.py",
        "filename": "screenshots/q4_exp3.png",
        "width": 860
    }
]

for item in items:
    feed = item.get("stdin_override", item["input"])
    res = subprocess.run(
        f"python3 {item['script']}",
        input=feed,
        text=True,
        capture_output=True,
        shell=True
    )
    render_terminal(
        title=item["title"],
        prompt_cmd=item["cmd"],
        input_text=item["input"],
        output_text=res.stdout,
        output_filename=item["filename"],
        width=item["width"]
    )

print("All screenshots generated successfully!")
