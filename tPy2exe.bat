rd /s /q build
rd /s /q dist
del *.exe
;pyinstaller -D -w -p C:\Python27\Lib; testDll.py
;pyinstaller -F -w -p C:\Python27\Lib; testDll.py
pyinstaller -D --console -p C:\Python27\Lib; testDll.py
pyinstaller -F --console -p C:\Python27\Lib; testDll.py -i credo.ico
timeout /t 1 /nobreak
copy dist\testDll.exe .\testDll.exe
rd /s /q build
rd /s /q dist
