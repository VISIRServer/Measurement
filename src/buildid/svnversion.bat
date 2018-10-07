SET SUBWCREVCMD="C:\Program Files\TortoiseSVN\bin\SubWCRev.exe"
if exist %SUBWCREVCMD% goto DO_VERSIONING

@echo #ifndef __SVN_VERSION_H__ > %3
@echo #define __SVN_VERSION_H__ >> %3
@echo // Autogenerated file, please do not edit
@echo // No version checker detected, SVN_VERSION is not defined >> %3
@echo #endif >> %3

exit

:DO_VERSIONING
%SUBWCREVCMD% %1 %2 %3