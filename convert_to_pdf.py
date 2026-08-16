import re
import html
import os

md_path = "/Users/mtn/code/mtg/ANLEITUNG.md"
html_path = "/Users/mtn/code/mtg/ANLEITUNG.html"

with open(md_path, "r", encoding="utf-8") as f:
    text = f.read()

# Simple, beautiful Markdown to HTML converter
def md_to_html(md):
    lines = md.split("\n")
    out = []
    in_code = False
    in_table = False
    in_list = False
    table_rows = []

    for line in lines:
        if line.startswith("```"):
            if in_code:
                out.append("</code></pre>")
                in_code = False
            else:
                lang = line[3:].strip()
                out.append(f'<pre><code class="language-{lang}">')
                in_code = True
            continue

        if in_code:
            out.append(html.escape(line))
            continue

        # Tables
        if line.startswith("|") and line.endswith("|"):
            if "---" in line:
                continue # separator
            cells = [c.strip() for c in line.strip("|").split("|")]
            if not in_table:
                in_table = True
                out.append("<table><thead><tr>")
                for c in cells:
                    out.append(f"<th>{inline_format(c)}</th>")
                out.append("</tr></thead><tbody>")
            else:
                out.append("<tr>")
                for c in cells:
                    out.append(f"<td>{inline_format(c)}</td>")
                out.append("</tr>")
            continue
        elif in_table:
            out.append("</tbody></table>")
            in_table = False

        # Headings
        if line.startswith("# "):
            out.append(f"<h1>{inline_format(line[2:])}</h1>")
        elif line.startswith("## "):
            out.append(f"<h2>{inline_format(line[3:])}</h2>")
        elif line.startswith("### "):
            out.append(f"<h3>{inline_format(line[4:])}</h3>")
        elif line.startswith("#### "):
            out.append(f"<h4>{inline_format(line[5:])}</h4>")
        elif line.startswith("> "):
            out.append(f"<blockquote>{inline_format(line[2:])}</blockquote>")
        elif line.startswith("* ") or line.startswith("- "):
            if not in_list:
                out.append("<ul>")
                in_list = True
            out.append(f"<li>{inline_format(line[2:])}</li>")
        elif line.startswith("1. ") or line.startswith("2. ") or line.startswith("3. ") or line.startswith("4. "):
            if not in_list:
                out.append("<ol>")
                in_list = True
            out.append(f"<li>{inline_format(line[3:])}</li>")
        elif line.strip() == "---":
            if in_list:
                out.append("</ul>" if "<ul>" in out[-2] else "</ol>")
                in_list = False
            out.append("<hr>")
        elif line.strip() == "":
            if in_list:
                out.append("</ul>" if "<ul>" in "".join(out[-5:]) else "</ol>")
                in_list = False
        else:
            out.append(f"<p>{inline_format(line)}</p>")

    if in_table:
        out.append("</tbody></table>")
    if in_list:
        out.append("</ul>")

    return "\n".join(out)

def inline_format(s):
    # bold
    s = re.sub(r'\*\*(.+?)\*\*', r'<strong>\1</strong>', s)
    # italic
    s = re.sub(r'\*(.+?)\*', r'<em>\1</em>', s)
    # inline code
    s = re.sub(r'`(.+?)`', r'<code>\1</code>', s)
    # links
    s = re.sub(r'\[(.+?)\]\((.+?)\)', r'<a href="\2">\1</a>', s)
    return s

body_html = md_to_html(text)

html_document = f"""<!DOCTYPE html>
<html lang="de">
<head>
<meta charset="UTF-8">
<title>Die Spiele-Schmiede: Zero to Hero</title>
<style>
  @import url('https://fonts.googleapis.com/css2?family=Outfit:wght@400;600;700;800&family=JetBrains+Mono:wght@400;600&display=swap');
  
  @page {{
    size: A4;
    margin: 20mm 15mm 20mm 15mm;
  }}

  body {{
    font-family: 'Outfit', -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif;
    line-height: 1.6;
    color: #1e293b;
    background: #ffffff;
    max-width: 860px;
    margin: 0 auto;
    padding: 30px 20px;
  }}

  h1 {{
    font-size: 2.2rem;
    font-weight: 800;
    color: #0f172a;
    border-bottom: 3px solid #3b82f6;
    padding-bottom: 12px;
    margin-top: 10px;
  }}

  h2 {{
    font-size: 1.5rem;
    font-weight: 700;
    color: #1e3a8a;
    margin-top: 35px;
    border-bottom: 1px solid #e2e8f0;
    padding-bottom: 6px;
    page-break-after: avoid;
  }}

  h3 {{
    font-size: 1.2rem;
    font-weight: 600;
    color: #1d4ed8;
    margin-top: 20px;
    page-break-after: avoid;
  }}

  p, li {{
    font-size: 1.05rem;
    color: #334155;
  }}

  code {{
    font-family: 'JetBrains Mono', Consolas, Monaco, monospace;
    background: #f1f5f9;
    color: #0369a1;
    padding: 2px 6px;
    border-radius: 4px;
    font-size: 0.92em;
  }}

  pre {{
    background: #0f172a;
    color: #f8fafc;
    padding: 16px 20px;
    border-radius: 8px;
    overflow-x: auto;
    font-family: 'JetBrains Mono', Consolas, Monaco, monospace;
    font-size: 0.95rem;
    line-height: 1.45;
    page-break-inside: avoid;
    box-shadow: 0 4px 6px -1px rgba(0, 0, 0, 0.1);
  }}

  pre code {{
    background: transparent;
    color: inherit;
    padding: 0;
  }}

  table {{
    width: 100%;
    border-collapse: collapse;
    margin: 20px 0;
    page-break-inside: avoid;
  }}

  th, td {{
    border: 1px solid #cbd5e1;
    padding: 10px 14px;
    text-align: left;
  }}

  th {{
    background: #f8fafc;
    font-weight: 700;
    color: #0f172a;
  }}

  tr:nth-child(even) {{
    background: #f8fafc;
  }}

  blockquote {{
    border-left: 4px solid #f59e0b;
    background: #fffbeb;
    padding: 12px 18px;
    margin: 18px 0;
    border-radius: 0 8px 8px 0;
    color: #92400e;
    font-weight: 600;
  }}

  hr {{
    border: none;
    border-top: 2px dashed #cbd5e1;
    margin: 30px 0;
  }}

  .print-bar {{
    background: #eff6ff;
    border: 1px solid #bfdbfe;
    padding: 12px 18px;
    border-radius: 8px;
    margin-bottom: 25px;
    display: flex;
    justify-content: space-between;
    align-items: center;
  }}

  .print-btn {{
    background: #2563eb;
    color: white;
    border: none;
    padding: 8px 18px;
    border-radius: 6px;
    font-weight: 600;
    cursor: pointer;
    font-family: inherit;
  }}
  .print-btn:hover {{
    background: #1d4ed8;
  }}

  @media print {{
    .print-bar {{
      display: none;
    }}
    body {{
      padding: 0;
    }}
    pre {{
      background: #1e293b !important;
      color: #f8fafc !important;
      -webkit-print-color-adjust: exact;
      print-color-adjust: exact;
    }}
  }}
</style>
</head>
<body>
<div class="print-bar">
  <span>💡 <strong>Tipp für PDF-Druck:</strong> Klicke auf den Button rechts oder drücke <kbd>⌘</kbd> + <kbd>P</kbd> und wähle <em>"Als PDF sichern"</em>.</span>
  <button class="print-btn" onclick="window.print()">📄 Als PDF drucken / speichern</button>
</div>

{body_html}

</body>
</html>
"""

with open(html_path, "w", encoding="utf-8") as f:
    f.write(html_document)

print(f"Generated: {html_path}")
