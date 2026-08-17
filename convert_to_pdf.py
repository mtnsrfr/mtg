import re
import html
import os

md_path = "/Users/mtn/code/mtg/ANLEITUNG.md"
html_path = "/Users/mtn/code/mtg/ANLEITUNG.html"

with open(md_path, "r", encoding="utf-8") as f:
    text = f.read()

def inline_format(s):
    # Escape HTML special characters in normal text first
    s = html.escape(s)
    
    # Inline code (un-escape inside code tag)
    def replace_code(m):
        code_text = m.group(1)
        return f'<code>{code_text}</code>'
    s = re.sub(r'`(.+?)`', replace_code, s)
    
    # Bold **text**
    s = re.sub(r'\*\*(.+?)\*\*', r'<strong>\1</strong>', s)
    # Italic *text*
    s = re.sub(r'\*(.+?)\*', r'<em>\1</em>', s)
    # Markdown links [text](url)
    s = re.sub(r'\[(.+?)\]\((.+?)\)', r'<a href="\2">\1</a>', s)
    return s

def md_to_html(md):
    lines = md.split("\n")
    out = []
    in_code = False
    code_lines = []
    code_lang = ""
    in_table = False
    list_stack = [] # tracks 'ul' or 'ol'

    def close_lists():
        nonlocal list_stack
        while list_stack:
            tag = list_stack.pop()
            out.append(f"</{tag}>")

    for line in lines:
        stripped = line.strip()

        # Check for Fenced Code Block start / end (supports any leading indentation)
        if stripped.startswith("```"):
            if in_code:
                # End of code block
                code_content = html.escape("\n".join(code_lines))
                out.append(f'<pre><code class="language-{code_lang}">{code_content}</code></pre>')
                in_code = False
                code_lines = []
                code_lang = ""
            else:
                # Start of code block
                in_code = True
                code_lang = stripped[3:].strip()
                code_lines = []
            continue

        if in_code:
            code_lines.append(line)
            continue

        # Tables
        if stripped.startswith("|") and stripped.endswith("|"):
            if "---" in stripped:
                continue # Skip markdown table separator row
            cells = [c.strip() for c in stripped.strip("|").split("|")]
            if not in_table:
                close_lists()
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

        # Empty lines
        if stripped == "":
            close_lists()
            continue

        # Horizontal rules
        if stripped == "---" or stripped == "***":
            close_lists()
            out.append("<hr>")
            continue

        # Headings
        if stripped.startswith("# "):
            close_lists()
            out.append(f"<h1>{inline_format(stripped[2:])}</h1>")
            continue
        elif stripped.startswith("## "):
            close_lists()
            out.append(f"<h2>{inline_format(stripped[3:])}</h2>")
            continue
        elif stripped.startswith("### "):
            close_lists()
            out.append(f"<h3>{inline_format(stripped[4:])}</h3>")
            continue
        elif stripped.startswith("#### "):
            close_lists()
            out.append(f"<h4>{inline_format(stripped[5:])}</h4>")
            continue

        # Blockquotes
        if stripped.startswith("> "):
            close_lists()
            out.append(f"<blockquote>{inline_format(stripped[2:])}</blockquote>")
            continue

        # Bullet Lists
        if stripped.startswith("* ") or stripped.startswith("- "):
            if not list_stack or list_stack[-1] != 'ul':
                close_lists()
                out.append("<ul>")
                list_stack.append('ul')
            out.append(f"<li>{inline_format(stripped[2:])}</li>")
            continue

        # Numbered Lists
        num_match = re.match(r'^(\d+)\.\s+(.*)$', stripped)
        if num_match:
            if not list_stack or list_stack[-1] != 'ol':
                close_lists()
                out.append("<ol>")
                list_stack.append('ol')
            out.append(f"<li>{inline_format(num_match.group(2))}</li>")
            continue

        # Normal Paragraphs
        close_lists()
        out.append(f"<p>{inline_format(stripped)}</p>")

    if in_code:
        code_content = html.escape("\n".join(code_lines))
        out.append(f'<pre><code class="language-{code_lang}">{code_content}</code></pre>')

    if in_table:
        out.append("</tbody></table>")

    close_lists()
    return "\n".join(out)

body_html = md_to_html(text)

html_document = f"""<!DOCTYPE html>
<html lang="de">
<head>
<meta charset="UTF-8">
<title>Die Spiele-Schmiede: Zero to Hero</title>
<style>
  @import url('https://fonts.googleapis.com/css2?family=Outfit:wght@400;500;600;700;800&family=JetBrains+Mono:wght@400;600;700&display=swap');
  
  @page {{
    size: A4;
    margin: 14mm 12mm 14mm 12mm;
  }}

  * {{
    box-sizing: border-box;
  }}

  body {{
    font-family: 'Outfit', -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif;
    line-height: 1.55;
    color: #1e293b;
    background: #ffffff;
    max-width: 100%;
    margin: 0 auto;
    padding: 10px 15px;
  }}

  h1 {{
    font-size: 2.1rem;
    font-weight: 800;
    color: #0f172a;
    border-bottom: 3px solid #3b82f6;
    padding-bottom: 10px;
    margin-top: 5px;
    margin-bottom: 18px;
  }}

  h2 {{
    font-size: 1.45rem;
    font-weight: 700;
    color: #1e3a8a;
    margin-top: 30px;
    margin-bottom: 12px;
    border-bottom: 1.5px solid #e2e8f0;
    padding-bottom: 5px;
    page-break-after: avoid;
  }}

  h3 {{
    font-size: 1.15rem;
    font-weight: 600;
    color: #1d4ed8;
    margin-top: 20px;
    margin-bottom: 8px;
    page-break-after: avoid;
  }}

  h4 {{
    font-size: 1.02rem;
    font-weight: 600;
    color: #334155;
    margin-top: 14px;
    margin-bottom: 6px;
    page-break-after: avoid;
  }}

  p {{
    font-size: 0.98rem;
    color: #334155;
    margin: 8px 0;
  }}

  ul, ol {{
    margin: 8px 0 14px 22px;
    padding: 0;
  }}

  li {{
    font-size: 0.98rem;
    color: #334155;
    margin-bottom: 5px;
  }}

  code {{
    font-family: 'JetBrains Mono', Consolas, Monaco, monospace;
    background: #eff6ff;
    color: #1d4ed8;
    padding: 2px 5px;
    border-radius: 4px;
    font-size: 0.88em;
    border: 1px solid #dbeafe;
  }}

  pre {{
    background: #0f172a;
    color: #f8fafc;
    padding: 12px 16px;
    border-radius: 6px;
    overflow-x: auto;
    font-family: 'JetBrains Mono', Consolas, Monaco, monospace;
    font-size: 0.86rem;
    line-height: 1.45;
    margin: 10px 0 14px 0;
    page-break-inside: avoid;
    box-shadow: 0 2px 4px rgba(0, 0, 0, 0.08);
    border-left: 4px solid #3b82f6;
    white-space: pre-wrap;
    word-break: break-word;
  }}

  pre code {{
    background: transparent;
    color: #f8fafc;
    padding: 0;
    border: none;
    font-size: inherit;
    white-space: pre-wrap;
    word-break: break-word;
  }}

  table {{
    width: 100%;
    border-collapse: collapse;
    margin: 16px 0;
    page-break-inside: avoid;
  }}

  th, td {{
    border: 1px solid #cbd5e1;
    padding: 8px 10px;
    text-align: left;
    font-size: 0.92rem;
  }}

  th {{
    background: #f1f5f9;
    font-weight: 700;
    color: #0f172a;
  }}

  tr:nth-child(even) {{
    background: #f8fafc;
  }}

  blockquote {{
    border-left: 4px solid #f59e0b;
    background: #fffbeb;
    padding: 10px 16px;
    margin: 14px 0;
    border-radius: 0 6px 6px 0;
    color: #92400e;
    font-weight: 600;
    font-size: 0.98rem;
  }}

  hr {{
    border: none;
    border-top: 2px dashed #cbd5e1;
    margin: 24px 0;
  }}

  a {{
    color: #2563eb;
    text-decoration: none;
    font-weight: 500;
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
    transition: background 0.15s ease;
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
      background: #0f172a !important;
      color: #f8fafc !important;
      -webkit-print-color-adjust: exact;
      print-color-adjust: exact;
    }}
    th {{
      background: #f1f5f9 !important;
      -webkit-print-color-adjust: exact;
      print-color-adjust: exact;
    }}
    blockquote {{
      background: #fffbeb !important;
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

print(f"Successfully generated: {html_path}")
