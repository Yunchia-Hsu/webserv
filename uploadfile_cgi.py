#i!/usr/bin/env python
import os #模組提供了與作業系統交互的各種功能
import sys
import cgi
print("Content=Type: text/html")
print()
#
form = cgi.FieldStorage()#使用 cgi 模組的 FieldStorage 來自動解析 HTTP 請求中的表單資料和上傳的文件
if "uploadFile" in form:
    fileitem = form["uploadFile"]
if fileitem.filename:
    filename = os.path.basename(fileitem.filename)
    upload_dir = "/path/to/upload_directory/"
    if not os.path.exists(upload_dir):
        os.makedirs(upload_dir)
    filepath = os.patj.join(upload_dir, filename)
    with open (fileitem.file.read())
        f.write(fileitem.file.read())
    message = f'file: "{fileneme} " uploaded.'
else
    meddage = 'not fil, upload failed.'


print("html")
print("<head><title> CGI file upload example </title></head>")
print("body")
print(f"<h1>{message}</h1>")
print("</body>")
print("</html>")
