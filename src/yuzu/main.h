// Copyright 2014 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <memory>
#include <optional>
#include <unordered_map>

#include <QMainWindow>
#include <QTimer>

#include "common/common_types.h"
#include "core/core.h"
#include "core/hle/service/acc/profile_manager.h"
#include "ui_main.h"
#include "yuzu/compatibility_list.h"
#include "yuzu/hotkeys.h"

class Config;
class EmuThread;
class GameList;
class GImageInfo;
class GRenderWindow;
class LoadingScreen;
class MicroProfileDialog;
class ProfilerWidget;
class QLabel;
class QPushButton;
class WaitTreeWidget;
enum class GameListOpenTarget;
class GameListPlaceholder;

namespace Core::Frontend {
struct SoftwareKeyboardParameters;
} // namespace Core::Frontend

namespace FileSys {
class ContentProvider;
class ManualContentProvider;
class VfsFilesystem;
} // namespace FileSys

enum class EmulatedDirectoryTarget {
    NAND,
    SDMC,
};

enum class ReinitializeKeyBehavior {
    NoWarning,
    Warning,
};

namespace DiscordRPC {
class DiscordInterface;
}

class GMainWindow : public QMainWindow {
    Q_OBJECT

    /// Max number of recently loaded items to keep track of
    static const int max_recent_files_item = 10;

    // TODO: Make use of this!
    enum {
        UI_IDLE,
        UI_EMU_BOOTING,
        UI_EMU_RUNNING,
        UI_EMU_STOPPING,
    };

public:
    void filterBarSetChecked(bool state);
    void UpdateUITheme();
    GMainWindow();
    ~GMainWindow() override;

    std::unique_ptr<DiscordRPC::DiscordInterface> discord_rpc;

    bool DropAction(QDropEvent* event);
    void AcceptDropEvent(QDropEvent* event);

signals:

    /**
     * Signal that is emitted when a new EmuThread has been created and an emulation session is
     * about to start. At this time, the core system emulation has been initialized, and all
     * emulation handles and memory should be valid.
     *
     * @param emu_thread Pointer to the newly created EmuThread (to be used by widgets that need to
     *      access/change emulation state).
     */
    void EmulationStarting(EmuThread* emu_thread);

    /**
     * Signal that is emitted when emulation is about to stop. At this time, the EmuThread and core
     * system emulation handles and memory are still valid, but are about become invalid.
     */
    void EmulationStopping();

    // Signal that tells widgets to update icons to use the current theme
    void UpdateThemedIcons();

    void ErrorDisplayFinished();

    void ProfileSelectorFinishedSelection(std::optional<Common::UUID> uuid);
    void SoftwareKeyboardFinishedText(std::optional<std::u16string> text);
    void SoftwareKeyboardFinishedCheckDialog();

    void WebBrowserUnpackRomFS();
    void WebBrowserFinishedBrowsing();

public slots:
    void OnLoadComplete();
    void ErrorDisplayDisplayError(QString body);
    void ProfileSelectorSelectProfile();
    void SoftwareKeyboardGetText(const Core::Frontend::SoftwareKeyboardParameters& parameters);
    void SoftwareKeyboardInvokeCheckDialog(std::u16string error_message);
    void WebBrowserOpenPage(std::string_view filename, std::string_view arguments);
    void OnAppFocusStateChanged(Qt::ApplicationState state);

private:
    void InitializeWidgets();
    void InitializeDebugWidgets();
    void InitializeRecentFileMenuActions();

    void SetDefaultUIGeometry();
    void RestoreUIState();

    void ConnectWidgetEvents();
    void ConnectMenuEvents();

    void PreventOSSleep();
    void AllowOSSleep();

    bool LoadROM(const QString& filename);
    void BootGame(const QString& filename);
    void ShutdownGame();

    void ShowTelemetryCallout();
    void SetDiscordEnabled(bool state);
    void LoadAmiibo(const QString& filename);

    void SelectAndSetCurrentUser();

    /**
     * Stores the filename in the recently loaded files list.
     * The new filename is stored at the beginning of the recently loaded files list.
     * After inserting the new entry, duplicates are removed meaning that if
     * this was inserted from \a OnMenuRecentFile(), the entry will be put on top
     * and remove from its previous position.
     *
     * Finally, this function calls \a UpdateRecentFiles() to update the UI.
     *
     * @param filename the filename to store
     */
    void StoreRecentFile(const QString& filename);

    /**
     * Updates the recent files menu.
     * Menu entries are rebuilt from the configuration file.
     * If there is no entry in the menu, the menu is greyed out.
     */
    void UpdateRecentFiles();

    /**
     * If the emulation is running,
     * asks the user if he really want to close the emulator
     *
     * @return true if the user confirmed
     */
    bool ConfirmClose();
    bool ConfirmChangeGame();
    bool ConfirmForceLockedExit();
    void RequestGameExit();
    void closeEvent(QCloseEvent* event) override;

private slots:
    void OnStartGame();
    void OnPauseGame();
    void OnStopGame();
    void OnMenuReportCompatibility();
    void OnOpenModsPage();
    void OnOpenQuickstartGuide();
    void OnOpenFAQ();
    /// Called whenever a user selects a game in the game list widget.
    void OnGameListLoadFile(QString game_path);
    void OnGameListOpenFolder(GameListOpenTarget target, const std::string& game_path);
    void OnTransferableShaderCacheOpenFile(u64 program_id);
    void OnGameListDumpRomFS(u64 program_id, const std::string& game_path);
    void OnGameListCopyTID(u64 program_id);
    void OnGameListNavigateToGamedbEntry(u64 program_id,
                                         const CompatibilityList& compatibility_list);
    void OnGameListOpenDirectory(const QString& directory);
    void OnGameListAddDirectory();
    void OnGameListShowList(bool show);
    void OnGameListOpenPerGameProperties(const std::string& file);
    void OnMenuLoadFile();
    void OnMenuLoadFolder();
    void OnMenuInstallToNAND();
    void OnMenuRecentFile();
    void OnConfigure();
    void OnLoadAmiibo();
    void OnOpenYuzuFolder();
    void OnAbout();
    void OnToggleFilterBar();
    void OnDisplayTitleBars(bool);
    void InitializeHotkeys();
    void ToggleFullscreen();
    void ShowFullscreen();
    void HideFullscreen();
    void ToggleWindowMode();
    void ResetWindowSize();
    void OnCaptureScreenshot();
    void OnCoreError(Core::System::ResultStatus, std::string);
    void OnReinitializeKeys(ReinitializeKeyBehavior behavior);

private:
    std::optional<u64> SelectRomFSDumpTarget(const FileSys::ContentProvider&, u64 program_id);
    void UpdateWindowTitle(const std::string& title_name = {},
                           const std::string& title_version = {});
    void UpdateStatusBar();
    void UpdateStatusButtons();
    void HideMouseCursor();
    void ShowMouseCursor();
    void OpenURL(const QUrl& url);

    Ui::MainWindow ui;

    GRenderWindow* render_window;
    GameList* game_list;
    LoadingScreen* loading_screen;

    GameListPlaceholder* game_list_placeholder;

    // Status bar elements
    QLabel* message_label = nullptr;
    QLabel* emu_speed_label = nullptr;
    QLabel* game_fps_label = nullptr;
    QLabel* emu_frametime_label = nullptr;
    QPushButton* async_status_button = nullptr;
    QPushButton* multicore_status_button = nullptr;
    QPushButton* renderer_status_button = nullptr;
    QPushButton* dock_status_button = nullptr;
    QTimer status_bar_update_timer;

    std::unique_ptr<Config> config;

    // Whether emulation is currently running in yuzu.
    bool emulation_running = false;
    std::unique_ptr<EmuThread> emu_thread;
    // The path to the game currently running
    QString game_path;

    bool auto_paused = false;
    QTimer mouse_hide_timer;

    // FS
    std::shared_ptr<FileSys::VfsFilesystem> vfs;
    std::unique_ptr<FileSys::ManualContentProvider> provider;

    // Debugger panes
    ProfilerWidget* profilerWidget;
    MicroProfileDialog* microProfileDialog;
    WaitTreeWidget* waitTreeWidget;

    QAction* actions_recent_files[max_recent_files_item];

    // stores default icon theme search paths for the platform
    QStringList default_theme_paths;

    HotkeyRegistry hotkey_registry;

protected:
    void dropEvent(QDropEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
};
