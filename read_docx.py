import zipfile
import re
import sys
import os

def get_docx_text(path, output_path):
    try:
        with zipfile.ZipFile(path) as document:
            xml_content = document.read('word/document.xml').decode('utf-8')
            # Remove XML tags
            text = re.sub('<[^<]+?>', '', xml_content)
            with open(output_path, 'w', encoding='utf-8') as f:
                f.write(text)
            print("Done")
    except Exception as e:
        print(str(e))

if __name__ == "__main__":
    if len(sys.argv) > 2:
        get_docx_text(sys.argv[1], sys.argv[2])
    else:
        print("Please provide input and output file paths.")
