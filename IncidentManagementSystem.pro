QT       += core gui sql widgets network printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = IncidentManagementSystem
TEMPLATE = app

SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/logindialog.cpp \
    src/incidentdetaildialog.cpp \
    src/dashboardwidget.cpp \
    src/russiantablemodel.cpp \
    src/orm/BaseModel.cpp \
    src/orm/Incident.cpp \
    src/orm/IncidentHistory.cpp \
    src/orm/User.cpp \
    src/modules/usermanagement.cpp \
    src/modules/exportmodule.cpp \
    src/modules/filtermodule.cpp \
    src/modules/managedfilterpanel.cpp \
    src/incidenthistorydialog.cpp

HEADERS += \
    include/mainwindow.h \
    include/logindialog.h \
    include/incidentdetaildialog.h \
    include/dashboardwidget.h \
    include/russiantablemodel.h \
    include/orm/BaseModel.h \
    include/orm/Incident.h \
    include/orm/IncidentHistory.h \
    include/orm/User.h \
    include/modules/usermanagement.h \
    include/modules/exportmodule.h \
    include/modules/filtermodule.h \
    include/modules/managedfilterpanel.h \
    include/incidenthistorydialog.h

INCLUDEPATH += include

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
