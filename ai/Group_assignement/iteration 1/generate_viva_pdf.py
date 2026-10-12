import os
import re
import markdown
import weasyprint
import latex2mathml.converter

def convert_latex_in_markdown(text):
    """
    Finds and converts all block math $$...$$ and inline math $...$
    into native MathML tags using latex2mathml, while protecting code blocks.
    """
    # 1. Protect fenced code blocks ```...```
    code_blocks = []
    def save_code_block(m):
        code_blocks.append(m.group(0))
        return f"___CODE_BLOCK_{len(code_blocks)-1}___"

    text = re.sub(r'```.*?```', save_code_block, text, flags=re.DOTALL)

    # 2. Protect inline code `...`
    inline_codes = []
    def save_inline_code(m):
        inline_codes.append(m.group(0))
        return f"___INLINE_CODE_{len(inline_codes)-1}___"

    text = re.sub(r'`[^`\n]+?`', save_inline_code, text)

    # 3. Convert block math $$...$$ to display MathML
    def convert_block_math(m):
        latex_str = m.group(1).strip()
        try:
            mathml = latex2mathml.converter.convert(latex_str, display="block")
            return f"\n\n<div class='math-block'>{mathml}</div>\n\n"
        except Exception as e:
            print(f"[Warning] Failed block math: {latex_str} -> {e}")
            return m.group(0)

    text = re.sub(r'\$\$(.*?)\$\$', convert_block_math, text, flags=re.DOTALL)

    # 4. Convert inline math $...$ to inline MathML
    def convert_inline_math(m):
        latex_str = m.group(1).strip()
        try:
            mathml = latex2mathml.converter.convert(latex_str, display="inline")
            return mathml
        except Exception as e:
            print(f"[Warning] Failed inline math: {latex_str} -> {e}")
            return m.group(0)

    text = re.sub(r'\$([^\$\n]+?)\$', convert_inline_math, text)

    # 5. Restore inline code
    for i, code in enumerate(inline_codes):
        text = text.replace(f"___INLINE_CODE_{i}___", code)

    # 6. Restore code blocks
    for i, block in enumerate(code_blocks):
        text = text.replace(f"___CODE_BLOCK_{i}___", block)

    return text

def build_pdf():
    # Read the markdown guide
    with open("VIVA_PREPARATION_GUIDE.md", "r", encoding="utf-8") as f:
        md_text = f.read()

    # Clean any long absolute paths into clean file names
    md_text = md_text.replace("file:///home/harshal/course/ai/Group_assignement/iteration%201/AI-GA1/q1.py", "`q1.py`")
    md_text = md_text.replace("file:///home/harshal/course/ai/Group_assignement/iteration%201/AI-GA1/q2.py", "`q2.py`")
    md_text = md_text.replace("file:///home/harshal/course/ai/Group_assignement/iteration%201/AI-GA1/q3.py", "`q3.py`")
    md_text = md_text.replace("file:///home/harshal/course/ai/Group_assignement/iteration%201/AI-GA1/q4.py", "`q4.py`")
    md_text = md_text.replace("file:///home/harshal/course/ai/Group_assignement/AI_GROUP_ASSIGNMENT_01_2026.pdf", "`AI_GROUP_ASSIGNMENT_01_2026.pdf`")
    md_text = md_text.replace("/home/harshal/course/ai/Group_assignement/", "")

    # Convert LaTeX formulas to native MathML
    print("Converting LaTeX equations to MathML...")
    md_text = convert_latex_in_markdown(md_text)

    # Convert markdown to html with tables and fenced code extensions
    html_body = markdown.markdown(
        md_text,
        extensions=["tables", "fenced_code", "nl2br", "sane_lists"]
    )

    # Style blockquotes as elegant callout cards
    html_body = html_body.replace(
        "<blockquote>\n<p><em>&quot;",
        "<div class='callout-box'><div class='callout-icon'>💡</div><div class='callout-content'><strong>Model Answer:</strong> &quot;"
    ).replace(
        "&quot;</em></p>\n</blockquote>",
        "&quot;</div></div>"
    )

    # Wrap Question headers
    html_body = html_body.replace("<strong>Q:", "<span class='q-badge'>VIVA QUESTION</span> <strong>")

    # Complete HTML Document with Print-Ready CSS & MathML Styling
    full_html = f"""<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<title>AI Group Assignment 01: Viva Voce Master Preparation Guide</title>
<style>
    @page {{
        size: A4;
        margin: 18mm 14mm 18mm 14mm;
        @top-left {{
            content: "Artificial Intelligence (AI-GA1) • Viva Voce Guide";
            font-size: 8pt;
            font-family: 'Helvetica Neue', Helvetica, Arial, sans-serif;
            color: #64748b;
            font-weight: 500;
        }}
        @top-right {{
            content: "Submission: q1.py | q2.py | q3.py | q4.py";
            font-size: 8pt;
            font-family: 'Helvetica Neue', Helvetica, Arial, sans-serif;
            color: #64748b;
        }}
        @bottom-left {{
            content: "Comprehensive Viva & Code Defense Handbook";
            font-size: 8pt;
            font-family: 'Helvetica Neue', Helvetica, Arial, sans-serif;
            color: #94a3b8;
        }}
        @bottom-right {{
            content: "Page " counter(page) " of " counter(pages);
            font-size: 8pt;
            font-family: 'Helvetica Neue', Helvetica, Arial, sans-serif;
            color: #475569;
            font-weight: 600;
        }}
    }}

    body {{
        font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, "Helvetica Neue", Arial, sans-serif;
        color: #1e293b;
        line-height: 1.55;
        font-size: 9.5pt;
        background: #ffffff;
    }}

    /* MathML Styling */
    math {{
        font-family: "DejaVu Math TeX Gyre", "STIX Two Math", "Cambria Math", "DejaVu Serif", serif;
        font-size: 1.05em;
        letter-spacing: 0.2px;
    }}

    .math-block {{
        text-align: center;
        margin: 12px 0 14px 0;
        padding: 9px 16px;
        background: #f8fafc;
        border: 1px solid #e2e8f0;
        border-radius: 6px;
        page-break-inside: avoid;
        box-shadow: 0 1px 2px rgba(0,0,0,0.03);
    }}

    .math-block math {{
        font-size: 1.15em;
        display: inline-block;
    }}

    /* Title Cover Header */
    .cover-banner {{
        background: linear-gradient(135deg, #0f172a 0%, #1e3a8a 60%, #2563eb 100%);
        color: #ffffff;
        padding: 22px 26px;
        border-radius: 10px;
        margin-bottom: 22px;
        box-shadow: 0 4px 12px rgba(15, 23, 42, 0.15);
    }}

    .cover-banner h1 {{
        font-size: 19pt;
        font-weight: 800;
        margin: 0 0 5px 0;
        color: #ffffff;
        letter-spacing: -0.5px;
        line-height: 1.2;
    }}

    .cover-banner .subtitle {{
        font-size: 10.5pt;
        color: #bfdbfe;
        margin: 0 0 12px 0;
        font-weight: 400;
    }}

    .meta-pills {{
        display: flex;
        flex-wrap: wrap;
        gap: 8px;
        margin-top: 8px;
    }}

    .pill {{
        background: rgba(255, 255, 255, 0.18);
        border: 1px solid rgba(255, 255, 255, 0.25);
        color: #f8fafc;
        padding: 3.5px 9px;
        border-radius: 9999px;
        font-size: 8pt;
        font-weight: 600;
    }}

    /* Typography */
    h1 {{
        color: #0f172a;
        font-size: 14.5pt;
        font-weight: 800;
        border-bottom: 2.5px solid #2563eb;
        padding-bottom: 5px;
        margin-top: 24px;
        margin-bottom: 12px;
        page-break-after: avoid;
    }}

    h2 {{
        color: #1e3a8a;
        font-size: 12pt;
        font-weight: 700;
        border-bottom: 1px solid #e2e8f0;
        padding-bottom: 4px;
        margin-top: 18px;
        margin-bottom: 9px;
        page-break-after: avoid;
    }}

    h3 {{
        color: #0369a1;
        font-size: 10.5pt;
        font-weight: 700;
        margin-top: 13px;
        margin-bottom: 6px;
        page-break-after: avoid;
    }}

    h4 {{
        color: #334155;
        font-size: 9.5pt;
        font-weight: 700;
        margin-top: 10px;
        margin-bottom: 4px;
        page-break-after: avoid;
    }}

    p {{
        margin: 0 0 8px 0;
    }}

    strong {{
        color: #0f172a;
        font-weight: 600;
    }}

    /* Tables */
    table {{
        width: 100%;
        border-collapse: collapse;
        margin: 12px 0 16px 0;
        font-size: 8.5pt;
        page-break-inside: avoid;
        box-shadow: 0 1px 3px rgba(0,0,0,0.05);
        border-radius: 6px;
        overflow: hidden;
    }}

    th {{
        background: #0f172a;
        color: #ffffff;
        font-weight: 700;
        text-align: left;
        padding: 7px 10px;
        border: 1px solid #1e293b;
        font-size: 8.5pt;
        letter-spacing: 0.2px;
    }}

    td {{
        padding: 6px 10px;
        border: 1px solid #cbd5e1;
        color: #334155;
        vertical-align: top;
    }}

    tr:nth-child(even) td {{
        background-color: #f8fafc;
    }}

    /* Code Blocks */
    pre {{
        background: #0f172a;
        color: #f8fafc;
        padding: 10px 14px;
        border-radius: 6px;
        font-family: "JetBrains Mono", "Fira Code", Consolas, Monaco, monospace;
        font-size: 8pt;
        line-height: 1.45;
        overflow-x: auto;
        margin: 8px 0 12px 0;
        page-break-inside: avoid;
        border-left: 4px solid #38bdf8;
    }}

    code {{
        font-family: "JetBrains Mono", "Fira Code", Consolas, Monaco, monospace;
        background: #f1f5f9;
        color: #0369a1;
        padding: 1.5px 4.5px;
        border-radius: 4px;
        font-size: 8.5pt;
        border: 1px solid #e2e8f0;
    }}

    pre code {{
        background: transparent;
        color: inherit;
        padding: 0;
        border: none;
        font-size: 8pt;
    }}

    /* Callout & Answer Boxes */
    .callout-box {{
        background: #f0fdf4;
        border: 1.5px solid #86efac;
        border-left: 5px solid #16a34a;
        border-radius: 6px;
        padding: 10px 14px;
        margin: 10px 0 14px 0;
        page-break-inside: avoid;
        display: flex;
        gap: 10px;
    }}

    .callout-icon {{
        font-size: 13pt;
        line-height: 1;
    }}

    .callout-content {{
        font-size: 9pt;
        color: #14532d;
        line-height: 1.5;
    }}

    .q-badge {{
        background: #ef4444;
        color: #ffffff;
        font-size: 7.5pt;
        font-weight: 800;
        padding: 2px 7px;
        border-radius: 4px;
        text-transform: uppercase;
        letter-spacing: 0.5px;
        margin-right: 4px;
        display: inline-block;
    }}

    /* Badges & Pills */
    .badge-opt {{
        background: #dcfce7;
        color: #166534;
        font-weight: 700;
        padding: 2px 6px;
        border-radius: 4px;
        font-size: 8pt;
    }}

    .badge-no-opt {{
        background: #fee2e2;
        color: #991b1b;
        font-weight: 700;
        padding: 2px 6px;
        border-radius: 4px;
        font-size: 8pt;
    }}

    /* Lists */
    ul, ol {{
        margin: 4px 0 10px 0;
        padding-left: 20px;
    }}

    li {{
        margin-bottom: 4px;
    }}

    hr {{
        border: none;
        border-top: 1px solid #cbd5e1;
        margin: 18px 0;
    }}
</style>
</head>
<body>

<div class="cover-banner">
    <h1>Artificial Intelligence — Viva Voce Master Guide</h1>
    <div class="subtitle">Complete Technical Walkthrough, Theoretical Foundations & Professor Q&A Defense</div>
    <div class="meta-pills">
        <span class="pill">Course: Artificial Intelligence</span>
        <span class="pill">Assignment: AI-GA1</span>
        <span class="pill">Codebase: q1.py | q2.py | q3.py | q4.py</span>
        <span class="pill">Topics: BFS, DFS, Greedy, A*, Minimax, Alpha-Beta, Hill Climbing</span>
    </div>
</div>

{html_body}

</body>
</html>
"""

    output_pdf_1 = "VIVA_PREPARATION_GUIDE.pdf"
    output_pdf_2 = "iteration 1/AI-GA1/VIVA_PREPARATION_GUIDE.pdf"

    print("Rendering PDF with WeasyPrint (with native MathML math rendering)...")
    weasyprint.HTML(string=full_html).write_pdf(output_pdf_1)
    print(f"Saved: {output_pdf_1}")

    weasyprint.HTML(string=full_html).write_pdf(output_pdf_2)
    print(f"Saved: {output_pdf_2}")

if __name__ == "__main__":
    build_pdf()
