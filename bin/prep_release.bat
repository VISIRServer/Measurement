@echo off
setlocal

set DIRLIST=conf;conf\maxlists;logs;savedcircuits

set MODULELIST=auth_ilabs;dbmysql;eqcom;gpibcontrol;ivicontrol

set FILELIST=libexpat.dll;measureserver_win.exe;measureserver_win.pdb;
set CONFFILES=conf\component.types;conf\flashpolicy.xml;conf\maxlists.conf;conf\measureserver.conf.dist;
set MAXFILES=conf\maxlists\DC6.max;conf\maxlists\DC25.max;conf\maxlists\Fgen.max;

set OUTPUTDIR=staging

rem echo %DIRLIST%
rem echo %OUTPUTDIR%

for %%v in ( %DIRLIST% ) do @mkdir %OUTPUTDIR%\%%v

for %%m in ( %MODULELIST% ) do (
	copy %%m.dll %OUTPUTDIR%\%%m.dll
	copy %%m.pdb %OUTPUTDIR%\%%m.pdb
)

for %%f in ( %FILELIST% %CONFFILES% %MAXFILES%) do copy %%f %OUTPUTDIR%\%%f


endlocal