/*
 * This file Copyright (C) 2009-2015 Mnemosyne LLC
 *
 * It may be used under the GNU GPL versions 2 or 3
 * or any future license endorsed by Mnemosyne LLC.
 *
 */

#pragma once

#include <QMap>

#include "BaseDialog.h"
#include "Prefs.h"

#include "ui_PrefsDialog.h"

class QHttp;
class QMessageBox;
class QString;

class Prefs;
class Session;

class PrefsDialog : public BaseDialog
{
    Q_OBJECT

public:
    PrefsDialog(Session&, Prefs&, QWidget* parent = nullptr);
    virtual ~PrefsDialog();

private:
    using key2widget_t = QMap<int, QWidget*>;

private:
    bool updateWidgetValue(QWidget* widget, int pref_key);
    void linkWidgetToPref(QWidget* widget, int pref_key);
    void updateBlocklistLabel();
    void updateDownloadingWidgetsLocality();
    void updateProxyValue(QWidget* widget, int pref_key);

    void setPref(int key, QVariant const& v);

    void initDownloadingTab();
    void initSeedingTab();
    void initSpeedTab();
    void initPrivacyTab();
    void initNetworkTab();
    void initProxyTab();
    void initDesktopTab();
    void initRemoteTab();

private slots:
    void checkBoxToggled(bool checked);
    void spinBoxEditingFinished();
    void timeEditingFinished();
    void lineEditingFinished();
    void pathChanged(QString const& path);
    void proxyTextChanged();
    void refreshPref(int key);
    void encryptionEdited(int);
    void altSpeedDaysEdited(int);
    void sessionUpdated();
    void onPortTested(bool);
    void onPortTest();
    void onUpdateCurrentExternalIP();
    void onIdleLimitChanged();
    void onQueueStalledMinutesChanged();

    void onUpdateBlocklistClicked();
    void onUpdateBlocklistCancelled();
    void onBlocklistDialogDestroyed(QObject*);
    void onBlocklistUpdated(int n);

private:
    Session& session_;
    Prefs& prefs_;

    Ui::PrefsDialog ui_ = {};

    bool const is_server_;
    bool is_local_ = {};

    key2widget_t widgets_;
    QWidgetList web_widgets_;
    QWidgetList web_auth_widgets_;
    QWidgetList web_whitelist_widgets_;
    QWidgetList proxy_widgets_;
    QWidgetList proxy_auth_widgets_;
    QWidgetList sched_widgets_;
    QWidgetList block_widgets_;
    QWidgetList unsupported_when_remote_;
    QWidgetList ext_ip_widgets_;

    int blocklist_http_tag_ = {};
    QHttp* blocklist_http_ = {};
    QMessageBox* blocklist_dialog_ = {};
    bool proxy_editor_active_ = {};
    int ext_ip_tick_count_ = {};
};
