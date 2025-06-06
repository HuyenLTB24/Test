import sys
import os
import time
from datetime import datetime
import re
import json

from PyQt6.QtCore import Qt, QSize, QThread, pyqtSignal, QTimer
from PyQt6.QtGui import QIcon, QFont, QColor, QBrush
from PyQt6.QtWidgets import (
    QApplication, QMainWindow, QWidget, QVBoxLayout, QHBoxLayout, 
    QLabel, QPushButton, QSlider, QFrame, QSizePolicy, QComboBox,
    QScrollArea, QGridLayout, QSplitter, QLineEdit, QTabWidget,
    QMessageBox, QGroupBox, QDialog, QFormLayout, QTableWidget, QTableWidgetItem,
    QTextEdit, QDateEdit, QCheckBox, QSpinBox, QHeaderView, QFileDialog
)

# Import TwitterBot class và các thành phần cần thiết
from twitter_bot import TwitterBot, DatabaseManager
from account_manager import AccountManager, AccountDialog

# Tạo class BotWorker để chạy bot trong thread riêng
class BotWorker(QThread):
    """Worker thread for running Twitter bot"""
    log_signal = pyqtSignal(str)
    tweet_processed_signal = pyqtSignal(str, str, str, str, str, bool, bool, bool, int, int, str, str)
    finished_signal = pyqtSignal()
    
    def __init__(self, credentials, main_window=None):
        super().__init__()
        self.credentials = credentials
        self.bot = None
        self.main_window = main_window
        
    def run(self):
        """Run bot in worker thread"""
        try:
            credentials_copy = self.credentials.copy()
            self.bot = TwitterBot(credentials_copy, main_window=self.main_window)
            
            # Connect signals
            self.bot.log_signal.connect(self.handle_log)
            self.bot.error_signal.connect(self.handle_error)
            self.bot.tweet_processed_signal.connect(self.handle_tweet_processed)
            
            # Start bot
            if self.bot.start():
                self.log_signal.emit("Bot đã khởi động thành công!")
            else:
                self.log_signal.emit("Không thể khởi động bot!")
                return
            
        except Exception as e:
            self.log_signal.emit(f"Lỗi trong BotWorker: {str(e)}")
            self.finished_signal.emit()
            
    def handle_log(self, message):
        """Handle log messages from bot"""
        self.log_signal.emit(message)
        
    def handle_error(self, message):
        """Handle error messages from bot"""
        self.log_signal.emit(f"ERROR: {message}")
        
    def handle_tweet_processed(self, 
                            time_str, username, tweet_url, reply_text, 
                            status, like, follow, retweet,
                            reply_count=0, view_count=0, tweet_id="", original_content=""):
        """Handle processed tweet signal from bot with extended statistics"""
        self.tweet_processed_signal.emit(
            time_str, username, tweet_url, reply_text, 
            status, like, follow, retweet,
            reply_count, view_count, tweet_id, original_content
        )
        
    def stop(self):
        """Stop the bot"""
        try:
            if self.bot:
                self.bot.cleanup()
                self.bot = None
        except Exception as e:
            self.log_signal.emit(f"Lỗi khi dừng bot: {str(e)}")
        self.finished_signal.emit()

class TwitterBotGUI(QMainWindow):
    def __init__(self):
        super().__init__()
        self.bot_instances = {}  # Lưu các instance bot theo profile_id
        self.bot_workers = {}    # Lưu các worker thread theo profile_id
        self.db = DatabaseManager(main_window=self)  # Khởi tạo DatabaseManager
        self.sidebar_menu_buttons = {}  # Store references to sidebar menu buttons
        self.initUI()
        self.showAccountsPage()
        # self.loadAccounts()  # XÓA hoặc COMMENT dòng này để không gọi khi layout chưa tạo
    
    def update_account_widget(self, profile_id, new_settings):
        """Update account widget with new settings from Account Manager"""
        try:
            self.log(f"Cập nhật cài đặt cho tài khoản {profile_id}")
            
            # Refresh account if it's currently active
            if profile_id in self.bot_instances:
                bot = self.bot_instances[profile_id]
                # Update bot settings if needed
                if hasattr(bot, 'update_settings'):
                    bot.update_settings(new_settings)
            
            # Reload the account display to show updated settings
            self.loadAccounts()
            
        except Exception as e:
            self.log(f"Lỗi khi cập nhật widget tài khoản: {str(e)}")
    
    def log(self, message):
        """Log a message to console and potentially to a log widget"""
        print(f"LOG: {message}")
        # If you want to add a log widget later, you can update it here
        
    def initUI(self):
        # Thiết lập cửa sổ chính
        self.setWindowTitle("Twitter Bot Management")
        self.setMinimumSize(1000, 600)
        
        # Widget chính
        main_widget = QWidget()
        main_layout = QHBoxLayout(main_widget)
        main_layout.setContentsMargins(0, 0, 0, 0)
        main_layout.setSpacing(0)
        
        # Tạo sidebar
        self.sidebar = self.create_sidebar()
        main_layout.addWidget(self.sidebar, 1)
        
        # Tạo area nội dung chính
        self.content_area = QWidget()
        self.content_layout = QVBoxLayout(self.content_area)
        self.content_layout.setContentsMargins(20, 20, 20, 20)
        main_layout.addWidget(self.content_area, 4)
        
        # Tạo header
        self.create_header()
        
        # Container cho account cards
        self.accounts_container = QWidget()
        self.accounts_layout = QGridLayout(self.accounts_container)
        self.accounts_layout.setContentsMargins(0, 0, 0, 0)
        self.accounts_layout.setSpacing(20)
        
        # Scroll area cho accounts
        scroll_area = QScrollArea()
        scroll_area.setWidgetResizable(True)
        scroll_area.setFrameShape(QFrame.Shape.NoFrame)
        scroll_area.setHorizontalScrollBarPolicy(Qt.ScrollBarPolicy.ScrollBarAlwaysOff)
        scroll_area.setWidget(self.accounts_container)
        
        self.content_layout.addWidget(scroll_area)
        
        # Mode selection
        self.mode_group = QComboBox()
        self.mode_group.addItems(["Feed Mode", "User Mode", "Comments Mode", "Trending Mode"])
        self.mode_group.currentIndexChanged.connect(self.onModeChanged)
        
        self.setCentralWidget(main_widget)
        
        # Style sheet
        self.setStyleSheet("""
            QMainWindow {
                background-color: #f2f9f1;
                color: #333;
            }
            QLabel {
                color: #333;
            }
            QPushButton {
                border: none;
                padding: 10px;
                border-radius: 5px;
            }
            QPushButton:hover {
                background-color: #e0f0e0;
            }
            QFrame#sidebar_frame {
                background-color: #ffffff;
                border-right: 1px solid #e0e0e0;
            }
            QFrame#account_card {
                background-color: white;
                border-radius: 10px;
                padding: 15px;
            }
            QFrame#account_card_content {
                background-color: white;
            }
            QTabWidget::pane {
                border: 1px solid #e0e0e0;
                border-radius: 5px;
            }
            QTabBar::tab {
                background-color: #f0f0f0;
                padding: 8px 15px;
                margin-right: 2px;
                border-top-left-radius: 4px;
                border-top-right-radius: 4px;
            }
            QTabBar::tab:selected {
                background-color: #4CAF50;
                color: white;
            }
            QComboBox {
                padding: 5px;
                border: 1px solid #e0e0e0;
                border-radius: 3px;
            }
            QLineEdit {
                padding: 5px;
                border: 1px solid #e0e0e0;
                border-radius: 3px;
            }
        """)
        
    def create_sidebar(self):
        # Frame cho sidebar
        sidebar_frame = QFrame()
        sidebar_frame.setObjectName("sidebar_frame")
        sidebar_layout = QVBoxLayout(sidebar_frame)
        sidebar_layout.setContentsMargins(0, 0, 0, 0)
        sidebar_layout.setSpacing(0)
        
        # Logo và label trong sidebar
        logo_container = QWidget()
        logo_layout = QHBoxLayout(logo_container)
        
        logo_label = QLabel("X")
        logo_label.setStyleSheet("background-color: #1DA1F2; color: white; font-size: 24px; padding: 10px; border-radius: 8px;")
        logo_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        logo_label.setFixedSize(60, 60)
        
        group_label = QLabel("Twitter Bot")
        group_label.setFont(QFont("Arial", 18, QFont.Weight.Bold))
        
        logo_layout.addWidget(logo_label)
        logo_layout.addWidget(group_label)
        logo_layout.setContentsMargins(20, 20, 20, 20)
        
        sidebar_layout.addWidget(logo_container)
        sidebar_layout.addSpacing(20)
        
        # Các menu item
        menu_items = [
            {"name": "Accounts", "icon": "👤", "selected": True},
            {"name": "Settings", "icon": "⚙️", "selected": False},
            {"name": "Statistics", "icon": "📊", "selected": False},
            {"name": "Log", "icon": "📝", "selected": False},  # Changed from "Templates" to "Log"
            {"name": "Account Manager", "icon": "👥", "selected": False}  # Thêm menu Account Manager
        ]
        
        for item in menu_items:
            menu_button = QPushButton(f"{item['icon']} {item['name']}")
            menu_button.setFont(QFont("Arial", 12))
            menu_button.setStyleSheet(f"""
                QPushButton {{
                    text-align: left;
                    padding: 15px 20px;
                    border-radius: 0px;
                    {'background-color: #e8f5e9; border-left: 5px solid #1DA1F2;' if item['selected'] else ''}
                }}
                QPushButton:hover {{
                    background-color: #e8f5e9;
                }}
            """)
            if item["name"] == "Accounts":
                menu_button.clicked.connect(self.showAccountsPage)
            elif item["name"] == "Settings":
                menu_button.clicked.connect(self.showSettingsPage)
            elif item["name"] == "Statistics":
                menu_button.clicked.connect(self.showStatsPage)
            elif item["name"] == "Log":
                menu_button.clicked.connect(self.showLogPage)
            elif item["name"] == "Account Manager":
                menu_button.clicked.connect(self.showAccountManager)  # Kết nối với hàm mới
            
            sidebar_layout.addWidget(menu_button)
            # Lưu trữ tham chiếu đến nút menu theo tên
            self.sidebar_menu_buttons[item["name"]] = menu_button
        
        sidebar_layout.addStretch(1)
        
        # Add bot controls at bottom of sidebar
        controls_container = QWidget()
        controls_layout = QVBoxLayout(controls_container)
        
        # Mode selector
        mode_label = QLabel("Bot Mode:")
        mode_label.setFont(QFont("Arial", 12, QFont.Weight.Bold))
        
        self.mode_combo = QComboBox()
        self.mode_combo.addItems(["Feed Mode", "User Mode", "Comments Mode", "Trending Mode"])
        self.mode_combo.setCurrentIndex(0)
        self.mode_combo.currentIndexChanged.connect(self.onGlobalModeChanged)
        
        controls_layout.addWidget(mode_label)
        controls_layout.addWidget(self.mode_combo)
        controls_layout.addSpacing(10)
        
        # Global control buttons
        global_buttons_container = QWidget()
        global_buttons_layout = QHBoxLayout(global_buttons_container)
        
        start_all_button = QPushButton("Start All")
        start_all_button.setStyleSheet("background-color: #4CD964; color: white;")
        start_all_button.clicked.connect(self.startAllBots)
        
        stop_all_button = QPushButton("Stop All")
        stop_all_button.setStyleSheet("background-color: #FF3B30; color: white;")
        stop_all_button.clicked.connect(self.stopAllBots)
        
        global_buttons_layout.addWidget(start_all_button)
        global_buttons_layout.addWidget(stop_all_button)
        
        controls_layout.addWidget(global_buttons_container)
        
        sidebar_layout.addWidget(controls_container)
        sidebar_layout.addSpacing(20)
        
        return sidebar_frame
        
    def create_header(self):
        # Header container
        header_widget = QWidget()
        header_layout = QVBoxLayout(header_widget)
        
        # Welcome title
        welcome_label = QLabel("Accounts Dashboard")
        welcome_label.setFont(QFont("Arial", 24, QFont.Weight.Bold))
        
        # Subtitle
        subtitle_label = QLabel("Manage your Twitter bot accounts and activities")
        subtitle_label.setFont(QFont("Arial", 14))
        subtitle_label.setStyleSheet("color: #757575")
        
        header_layout.addWidget(welcome_label)
        header_layout.addWidget(subtitle_label)
        
        # Thêm nút "Add Account" vào header
        add_account_button = QPushButton("Add Account")
        add_account_button.setStyleSheet("""
            background-color: #1DA1F2;
            color: white;
            padding: 8px 15px;
            border-radius: 5px;
            font-weight: bold;
        """)
        add_account_button.setFixedWidth(150)
        add_account_button.clicked.connect(self.addNewAccount)
        header_layout.addWidget(add_account_button, 0, Qt.AlignmentFlag.AlignRight)
        
        self.content_layout.addWidget(header_widget)
    
    def loadAccounts(self):
        """Load tài khoản từ database"""
        try:
            # Xóa tất cả account hiện tại trong grid
            for i in reversed(range(self.accounts_layout.count())):
                widget = self.accounts_layout.itemAt(i).widget()
                if widget:
                    widget.deleteLater()
            
            # Lấy danh sách account từ database
            accounts = self.db.get_all_accounts()  # Sửa thành get_all_accounts thay vì get_accounts
            
            if not accounts:
                self.showMessage("Không có tài khoản nào", "Vui lòng thêm tài khoản trong phần Account Manager")
                return
                
            # Thêm mỗi account vào grid
            for i, account in enumerate(accounts):
                profile_id = account.get('profile_id')
                
                # Tạo account card
                account_data = {
                    "name": account.get('name', f"Account {i+1}"),
                    "username": account.get('username', ""),
                    "profile_id": profile_id,
                    "status": "Stopped",
                    "replies": 0,
                    "likes": 0,
                    "follows": 0,
                    "gemini_key": account.get('gemini_key', ""),
                    "chatgpt_key": account.get('chatgpt_key', ""),
                    "use_gemini": account.get('use_gemini', True)
                }
                
                # Lấy số liệu thống kê từ database
                stats = self.db.get_account_stats(profile_id)
                if stats:
                    account_data["replies"] = stats.get('replies_sent', 0)
                    account_data["likes"] = stats.get('likes_given', 0)
                    account_data["follows"] = stats.get('follows_made', 0)
                
                # Thêm vào grid
                row, col = divmod(i, 4)
                account_card = self.create_account_card(account_data)
                account_card.setProperty("profile_id", profile_id)  # Lưu profile_id vào card
                self.accounts_layout.addWidget(account_card, row, col)
                
                # Nếu bot đã được khởi tạo trước đó, cập nhật status
                if profile_id in self.bot_instances:
                    bot = self.bot_instances[profile_id]
                    if bot.is_paused:
                        self.updateAccountStatus(profile_id, "Paused")
                    else:
                        self.updateAccountStatus(profile_id, "Running")
                
        except Exception as e:
            self.showMessage("Lỗi", f"Không thể tải danh sách tài khoản: {str(e)}")
    
    def create_account_card(self, account_data):
        # Frame cho account card
        card_frame = QFrame()
        card_frame.setObjectName("account_card")
        card_frame.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Preferred)
        
        card_layout = QVBoxLayout(card_frame)
        card_layout.setContentsMargins(15, 15, 15, 15)
        
        profile_id = account_data.get("profile_id", "")
        
        # Header với icon và tên tài khoản
        header_widget = QWidget()
        header_layout = QHBoxLayout(header_widget)
        header_layout.setContentsMargins(0, 0, 0, 0)
        
        icon_label = QLabel("👤")
        icon_label.setFont(QFont("Arial", 16))
        
        name_label = QLabel(account_data["username"])
        name_label.setFont(QFont("Arial", 14, QFont.Weight.Bold))
        
        # Switch status
        switch_color = "#4CD964" if account_data["status"] == "Running" else "#8E8E93"
        switch = QPushButton()
        switch.setObjectName(f"switch_{profile_id}")
        switch.setCheckable(True)
        switch.setChecked(account_data["status"] == "Running")
        switch.setFixedSize(50, 30)
        switch.setStyleSheet(f"""
            QPushButton {{
                background-color: {switch_color};
                border-radius: 15px;
                border: none;
            }}
            QPushButton::checked {{
                background-color: #4CD964;
            }}
            QPushButton::!checked {{
                background-color: #8E8E93;
            }}
        """)
        
        # Kết nối signal
        switch.clicked.connect(lambda checked, pid=profile_id: self.toggleBot(pid, checked))
        
        header_layout.addWidget(icon_label)
        header_layout.addWidget(name_label, 1)
        header_layout.addWidget(switch)
        
        card_layout.addWidget(header_widget)
        
        # Username and status
        username_label = QLabel(f"@{account_data['username']}")
        username_label.setStyleSheet("color: #1DA1F2;")
        username_label.setFont(QFont("Arial", 12))
        card_layout.addWidget(username_label)
        
        status_label = QLabel(account_data["status"])
        status_label.setObjectName(f"status_{profile_id}")
        status_label.setStyleSheet(
            "color: #4CD964;" if account_data["status"] == "Running" else "color: #8E8E93;"
        )
        card_layout.addWidget(status_label)
        
        # Tabs for activities, settings, etc
        tabs = QTabWidget()
        
        # Stats tab
        stats_tab = QWidget()
        stats_layout = QVBoxLayout(stats_tab)
        
        # Stats info display
        stats_widget = QWidget()
        stats_grid = QGridLayout(stats_widget)
        stats_grid.setContentsMargins(0, 0, 0, 0)
        
        replies_label = QLabel(f"Replies: {account_data['replies']}")
        replies_label.setObjectName(f"replies_{profile_id}")
        
        likes_label = QLabel(f"Likes: {account_data['likes']}")
        likes_label.setObjectName(f"likes_{profile_id}")
        
        follows_label = QLabel(f"Follows: {account_data['follows']}")
        follows_label.setObjectName(f"follows_{profile_id}")
        
        stats_grid.addWidget(replies_label, 0, 0)
        stats_grid.addWidget(likes_label, 1, 0)
        stats_grid.addWidget(follows_label, 2, 0)
        
        # Performance stats
        performance_widget = QWidget()
        performance_layout = QVBoxLayout(performance_widget)
        performance_layout.setContentsMargins(0, 0, 0, 0)
        
        performance_value = QLabel("Active" if account_data["status"] == "Running" else "Inactive")
        performance_value.setObjectName(f"performance_{profile_id}")
        performance_value.setFont(QFont("Arial", 18, QFont.Weight.Bold))
        performance_value.setStyleSheet("color: #4CD964;" if account_data["status"] == "Running" else "color: #FF3B30;")
        
        performance_layout.addWidget(performance_value, 0, Qt.AlignmentFlag.AlignRight)
        
        stats_grid.addWidget(performance_widget, 0, 1, 3, 1)
        stats_layout.addWidget(stats_widget)
        
        # Settings tab
        settings_tab = QWidget()
        settings_layout = QVBoxLayout(settings_tab)
        
        # Mode settings
        mode_container = QWidget()
        mode_layout = QHBoxLayout(mode_container)
        
        mode_label = QLabel("Mode:")
        mode_combo = QComboBox()
        mode_combo.setObjectName(f"mode_combo_{profile_id}")
        mode_combo.addItems(["Feed Mode", "User Mode", "Comments Mode", "Trending Mode"])
        
        # Kết nối signal
        mode_combo.currentIndexChanged.connect(lambda idx, pid=profile_id: self.onAccountModeChanged(pid, idx))
        
        mode_layout.addWidget(mode_label)
        mode_layout.addWidget(mode_combo, 1)
        
        # Search keyword for trending mode
        keyword_container = QWidget()
        keyword_layout = QHBoxLayout(keyword_container)
        
        keyword_label = QLabel("Keyword:")
        keyword_input = QLineEdit()
        keyword_input.setObjectName(f"keyword_input_{profile_id}")
        keyword_input.setPlaceholderText("Enter search keyword")
        
        keyword_layout.addWidget(keyword_label)
        keyword_layout.addWidget(keyword_input, 1)
        
        settings_layout.addWidget(mode_container)
        settings_layout.addWidget(keyword_container)
        
        # Thêm nút config chi tiết
        config_btn = QPushButton("Advanced Settings")
        config_btn.setStyleSheet("background-color: #1DA1F2; color: white;")
        config_btn.clicked.connect(lambda: self.showAccountSettings(profile_id))
        settings_layout.addWidget(config_btn)
        
        settings_layout.addStretch(1)
        
        # Add tabs
        tabs.addTab(stats_tab, "Stats")
        tabs.addTab(settings_tab, "Settings")
        
        card_layout.addWidget(tabs)
        
        # Buttons
        buttons_widget = QWidget()
        buttons_layout = QHBoxLayout(buttons_widget)
        buttons_layout.setContentsMargins(0, 10, 0, 0)
        
        start_button = QPushButton("Start")
        start_button.setStyleSheet("background-color: #4CD964; color: white;")
        start_button.clicked.connect(lambda checked, pid=profile_id: self.startBot(pid))
        
        stop_button = QPushButton("Stop")
        stop_button.setStyleSheet("background-color: #FF3B30; color: white;")
        stop_button.clicked.connect(lambda checked, pid=profile_id: self.stopBot(pid))
        
        pause_button = QPushButton("Pause")
        pause_button.setStyleSheet("background-color: #FF9500; color: white;")
        pause_button.clicked.connect(lambda checked, pid=profile_id: self.pauseBot(pid))
        
        edit_button = QPushButton("Edit")
        edit_button.setStyleSheet("background-color: #1DA1F2; color: white;")
        edit_button.clicked.connect(lambda checked, pid=profile_id: self.editAccount(pid))
        
        buttons_layout.addWidget(start_button)
        buttons_layout.addWidget(stop_button)
        buttons_layout.addWidget(pause_button)
        buttons_layout.addWidget(edit_button)
        
        card_layout.addWidget(buttons_widget)
        
        return card_frame

    # Thêm phương thức để hiển thị AccountManager
    def showAccountManager(self):
        """Hiển thị giao diện Account Manager"""
        try:
            # Update the active menu item
            self.set_active_menu_item("Account Manager")
            
            account_manager = AccountManager(main_window=self)
            account_manager.account_changed.connect(self.onAccountChanged)
            account_manager.exec()
            # Sau khi đóng AccountManager, refresh lại danh sách tài khoản
            self.loadAccounts()
        except Exception as e:
            self.showMessage("Lỗi", f"Không thể mở Account Manager: {str(e)}")

    def onAccountChanged(self, profile_id):
        """Xử lý khi thông tin tài khoản thay đổi"""
        self.loadAccounts()  # Tải lại tất cả tài khoản

    def addNewAccount(self):
        """Thêm tài khoản mới"""
        try:
            dialog = AccountDialog(self)
            if dialog.exec() == QDialog.DialogCode.Accepted:
                # Lấy thông tin từ dialog
                account_data = {
                    'profile_id': dialog.profile_id_input.text().strip(),
                    'username': dialog.username_input.text().strip(),
                    'gemini_key': dialog.gemini_key.text().strip(),
                    'chatgpt_key': dialog.chatgpt_key.text().strip(),
                    'use_gemini': dialog.gemini_radio.isChecked()
                }
                
                # Kiểm tra dữ liệu
                if not account_data['profile_id'] or not account_data['username']:
                    self.showMessage("Lỗi", "Profile ID và Username không được để trống")
                    return
                    
                # Thêm vào database
                if self.db.add_account(account_data):
                    # Refresh danh sách tài khoản
                    self.loadAccounts()
                    
                    # Thông báo thành công
                    self.showMessage("Thành công", f"Đã thêm tài khoản @{account_data['username']}")
                else:
                    self.showMessage("Lỗi", "Không thể thêm tài khoản")
                    
        except Exception as e:
            self.showMessage("Lỗi", f"Không thể thêm tài khoản: {str(e)}")
            
    def showAccountSettings(self, profile_id):
        """Hiển thị dialog cấu hình chi tiết cho bot"""
        try:
            # Lấy thông tin tài khoản và settings từ database
            account = self.db.get_account(profile_id)
            if not account:
                self.showMessage("Lỗi", "Không tìm thấy thông tin tài khoản")
                return
                
            username = account.get('username', '')
            settings = self.db.get_settings(profile_id)
            
            # Show account settings dialog from AccountManager
            account_manager = AccountManager(main_window=self)
            account_manager.show_account_settings(profile_id)
            
            # Refresh lại tài khoản sau khi cập nhật settings
            self.loadAccounts()
            
        except Exception as e:
            self.showMessage("Lỗi", f"Không thể mở cấu hình tài khoản: {str(e)}")
    
    def editAccount(self, profile_id):
        """Sửa thông tin tài khoản"""
        try:
            # Lấy thông tin tài khoản từ database
            account = self.db.get_account(profile_id)
            if not account:
                self.showMessage("Lỗi", "Không tìm thấy thông tin tài khoản")
                return
                
            # Hiển thị dialog chỉnh sửa
            dialog = AccountDialog(self)
            dialog.profile_id_input.setText(profile_id)
            dialog.profile_id_input.setReadOnly(True)  # Profile ID không thể thay đổi
            dialog.username_input.setText(account.get('username', ''))
            dialog.gemini_key.setText(account.get('gemini_key', ''))
            dialog.chatgpt_key.setText(account.get('chatgpt_key', ''))
            if account.get('use_gemini', True):
                dialog.gemini_radio.setChecked(True)
            else:
                dialog.chatgpt_radio.setChecked(True)
            
            # Xử lý khi người dùng chấp nhận thay đổi
            if dialog.exec() == QDialog.DialogCode.Accepted:
                # Lấy thông tin mới
                updated_account = {
                    'profile_id': dialog.profile_id_input.text().strip(),
                    'username': dialog.username_input.text().strip(),
                    'gemini_key': dialog.gemini_key.text().strip(),
                    'chatgpt_key': dialog.chatgpt_key.text().strip(),
                    'use_gemini': dialog.gemini_radio.isChecked()
                }
                
                # Kiểm tra dữ liệu
                if not updated_account['username']:
                    self.showMessage("Lỗi", "Username không được để trống")
                    return
                    
                # Cập nhật vào database
                if self.db.add_account(updated_account):
                    # Nếu bot đang chạy, khởi động lại
                    if profile_id in self.bot_instances:
                        self.stopBot(profile_id)
                        self.startBot(profile_id)
                    
                    # Refresh danh sách tài khoản
                    self.loadAccounts()
                    
                    # Thông báo thành công
                    self.showMessage("Thành công", f"Đã cập nhật tài khoản @{updated_account['username']}")
                else:
                    self.showMessage("Lỗi", "Không thể cập nhật tài khoản")
                
        except Exception as e:
            self.showMessage("Lỗi", f"Không thể chỉnh sửa tài khoản: {str(e)}")

    def startBot(self, profile_id):
        """Khởi động bot cho tài khoản"""
        try:
            # Kiểm tra xem bot đã được khởi tạo chưa
            if profile_id not in self.bot_instances:
                # Lấy thông tin tài khoản từ database
                account = self.db.get_account(profile_id)
                if not account:
                    self.showMessage("Lỗi", f"Không tìm thấy thông tin tài khoản {profile_id}")
                    return
                
                # Khởi tạo bot
                credentials = {
                    'username': account.get('username', ''),
                    'profile_id': profile_id,
                    'use_gemini': account.get('use_gemini', True),
                    'gemini_key': account.get('gemini_key', ''),
                    'chatgpt_key': account.get('chatgpt_key', '')
                }
                
                bot = TwitterBot(credentials, main_window=self)
                
                # Kết nối signal
                bot.tweet_processed_signal.connect(self.onTweetProcessed)
                bot.log_signal.connect(self.onBotLog)
                bot.error_signal.connect(self.onBotError)
                
                # Lưu vào dictionary
                self.bot_instances[profile_id] = bot
                
                # Set mode cho bot
                mode_combo = self.findChild(QComboBox, f"mode_combo_{profile_id}")
                if mode_combo:
                    mode_index = mode_combo.currentIndex() + 1  # mode_id từ 1-4
                    bot.mode_id = mode_index
            
            bot = self.bot_instances[profile_id]
            
            # Kiểm tra trạng thái pause
            if hasattr(bot, 'is_paused') and bot.is_paused:
                bot.is_paused = False
                bot.resume()
                self.updateAccountStatus(profile_id, "Running")
                return
                
            # Khởi động bot trong thread riêng
            worker = BotWorker(bot, main_window=self)
            worker.finished.connect(lambda: self.onBotFinished(profile_id))
            
            # Lưu worker và khởi động
            self.bot_workers[profile_id] = worker
            worker.start()
            
            # Cập nhật trạng thái UI
            self.updateAccountStatus(profile_id, "Running")
            
        except Exception as e:
            self.showMessage("Lỗi", f"Không thể khởi động bot: {str(e)}")
    
    def stopBot(self, profile_id):
        """Dừng bot đang chạy"""
        try:
            if profile_id in self.bot_instances:
                bot = self.bot_instances[profile_id]
                
                # Dừng worker thread
                if profile_id in self.bot_workers:
                    worker = self.bot_workers[profile_id]
                    worker.force_stop = True
                    worker.terminate()
                    worker.wait(1000)  # Đợi tối đa 1 giây
                    
                    # Xóa worker
                    del self.bot_workers[profile_id]
                
                # Dọn dẹp tài nguyên bot
                bot.cleanup()
                
                # Cập nhật UI
                self.updateAccountStatus(profile_id, "Stopped")
                
        except Exception as e:
            self.showMessage("Lỗi", f"Không thể dừng bot: {str(e)}")
    
    def pauseBot(self, profile_id):
        """Tạm dừng bot"""
        try:
            if profile_id in self.bot_instances:
                bot = self.bot_instances[profile_id]
                
                # Đặt flag pause
                bot.is_paused = True
                
                # Gọi hàm pause
                if hasattr(bot, 'pause_activities'):
                    bot.pause_activities()
                
                # Cập nhật UI
                self.updateAccountStatus(profile_id, "Paused")
                
        except Exception as e:
            self.showMessage("Lỗi", f"Không thể tạm dừng bot: {str(e)}")
    
    def toggleBot(self, profile_id, checked):
        """Toggle trạng thái bot theo switch"""
        if checked:
            self.startBot(profile_id)
        else:
            self.stopBot(profile_id)
    
    def configBot(self, profile_id):
        """Mở cửa sổ cấu hình chi tiết cho bot"""
        # Thông báo chức năng chưa được cài đặt
        self.showMessage("Thông báo", "Chức năng cấu hình chi tiết đang được phát triển")
    
    def onAccountModeChanged(self, profile_id, mode_index):
        """Xử lý khi thay đổi mode của một tài khoản"""
        try:
            if profile_id in self.bot_instances:
                bot = self.bot_instances[profile_id]
                
                # mode_id từ 1-4
                bot.mode_id = mode_index + 1
                
                # Hiện/ẩn trường keyword tùy theo mode
                keyword_input = self.findChild(QLineEdit, f"keyword_input_{profile_id}")
                if keyword_input:
                    # Chỉ hiển thị keyword input cho trending mode (index = 3)
                    keyword_input.setVisible(mode_index == 3)
                    
        except Exception as e:
            self.showMessage("Lỗi", f"Không thể thay đổi mode: {str(e)}")
    
    def onGlobalModeChanged(self, mode_index):
        """Thay đổi mode cho tất cả các bot"""
        try:
            # Duyệt qua tất cả combo box mode trong các account card
            for profile_id in self.bot_instances:
                mode_combo = self.findChild(QComboBox, f"mode_combo_{profile_id}")
                if mode_combo:
                    mode_combo.setCurrentIndex(mode_index)
                
        except Exception as e:
            self.showMessage("Lỗi", f"Không thể thay đổi mode toàn cục: {str(e)}")
    
    def startAllBots(self):
        """Khởi động tất cả các bot"""
        try:
            # Lấy danh sách tài khoản từ database
            accounts = self.db.get_all_accounts()  # Sửa thành get_all_accounts thay vì get_accounts
            
            for account in accounts:
                profile_id = account.get('profile_id')
                self.startBot(profile_id)
                
            self.showMessage("Thành công", "Đã khởi động tất cả các bot")
                
        except Exception as e:
            self.showMessage("Lỗi", f"Không thể khởi động tất cả bot: {str(e)}")
    
    def stopAllBots(self):
        """Dừng tất cả các bot"""
        try:
            # Lấy danh sách bot đang chạy
            for profile_id in list(self.bot_instances.keys()):
                self.stopBot(profile_id)
                
            self.showMessage("Thành công", "Đã dừng tất cả các bot")
                
        except Exception as e:
            self.showMessage("Lỗi", f"Không thể dừng tất cả bot: {str(e)}")
    
    def updateAccountStatus(self, profile_id, status):
        """Cập nhật trạng thái hiển thị của tài khoản"""
        try:
            # Cập nhật label status
            status_label = self.findChild(QLabel, f"status_{profile_id}")
            if status_label:
                status_label.setText(status)
                if status == "Running":
                    status_label.setStyleSheet("color: #4CD964;")
                elif status == "Paused":
                    status_label.setStyleSheet("color: #FF9500;")
                else:  # Stopped
                    status_label.setStyleSheet("color: #8E8E93;")
            
            # Cập nhật performance
            performance_label = self.findChild(QLabel, f"performance_{profile_id}")
            if performance_label:
                if status == "Running":
                    performance_label.setText("Active")
                    performance_label.setStyleSheet("color: #4CD964;")
                elif status == "Paused":
                    performance_label.setText("Paused")
                    performance_label.setStyleSheet("color: #FF9500;")
                else:  # Stopped
                    performance_label.setText("Inactive")
                    performance_label.setStyleSheet("color: #FF3B30;")
            
            # Cập nhật switch
            switch = self.findChild(QPushButton, f"switch_{profile_id}")
            if switch:
                switch.setChecked(status == "Running")
                if status == "Running":
                    switch.setStyleSheet("QPushButton { background-color: #4CD964; border-radius: 15px; border: none; }")
                else:
                    switch.setStyleSheet("QPushButton { background-color: #8E8E93; border-radius: 15px; border: none; }")
                
        except Exception as e:
            self.showMessage("Lỗi", f"Không thể cập nhật trạng thái: {str(e)}")
    
    def onTweetProcessed(self, time_str, username, tweet_link, reply_text, status, like_success, follow_success, retweet_success, response_time, char_count, additional_stats_json, current_url):
        """Handle tweet processed signal and update statistics"""
        try:
            # Parse additional stats
            additional_stats = json.loads(additional_stats_json) if additional_stats_json else {}
            
            # Update metrics
            if self.metrics_cards:
                # Increment total processed
                total_processed = int(self.metrics_cards["total_processed"].text()) + 1
                self.metrics_cards["total_processed"].setText(str(total_processed))
                
                # Update success rate
                success_count = int(self.metrics_cards["success_rate"].text().rstrip('%')) * total_processed / 100
                if status == "Thành công":
                    success_count += 1
                self.metrics_cards["success_rate"].setText(f"{(success_count/total_processed*100):.1f}%")
                
                # Update average response time
                current_avg = float(self.metrics_cards["avg_response"].text().rstrip('ms'))
                new_avg = (current_avg * (total_processed - 1) + response_time) / total_processed
                self.metrics_cards["avg_response"].setText(f"{new_avg:.0f}ms")
                
                # Update verified users count
                if additional_stats.get("is_verified"):
                    verified_count = int(self.metrics_cards["verified_users"].text()) + 1
                    self.metrics_cards["verified_users"].setText(str(verified_count))
                
                # Update media tweets count
                if additional_stats.get("has_media"):
                    media_count = int(self.metrics_cards["media_tweets"].text()) + 1
                    self.metrics_cards["media_tweets"].setText(str(media_count))
                
                # Update average interactions
                current_avg_interactions = float(self.metrics_cards["avg_interactions"].text())
                new_interactions = additional_stats.get("interaction_count", 0)
                new_avg_interactions = (current_avg_interactions * (total_processed - 1) + new_interactions) / total_processed
                self.metrics_cards["avg_interactions"].setText(f"{new_avg_interactions:.1f}")
            
            # Add row to table
            row = self.tweet_table.rowCount()
            self.tweet_table.insertRow(row)
            
            # Set row data
            self.tweet_table.setItem(row, 0, QTableWidgetItem(time_str))
            self.tweet_table.setItem(row, 1, QTableWidgetItem(f"@{username}"))
            self.tweet_table.setItem(row, 2, QTableWidgetItem(additional_stats.get("username", "unknown")))
            self.tweet_table.setItem(row, 3, QTableWidgetItem(additional_stats.get("content", "")))
            self.tweet_table.setItem(row, 4, QTableWidgetItem(f"{response_time}ms"))
            self.tweet_table.setItem(row, 5, QTableWidgetItem(status))
            self.tweet_table.setItem(row, 6, QTableWidgetItem("Yes" if additional_stats.get("is_verified") else "No"))
            self.tweet_table.setItem(row, 7, QTableWidgetItem("Yes" if additional_stats.get("has_media") else "No"))
            
            # Scroll to new row
            self.tweet_table.scrollToBottom()
            
        except Exception as e:
            print(f"Error updating tweet statistics: {str(e)}")
    
    def onBotLog(self, message):
        """Xử lý log từ bot"""
        print(f"Bot log: {message}")
    
    def onBotError(self, message):
        """Xử lý lỗi từ bot"""
        print(f"Bot error: {message}")
    
    def onBotFinished(self, profile_id):
        """Xử lý khi bot kết thúc"""
        try:
            # Kiểm tra nếu bot dừng do lỗi
            if profile_id in self.bot_instances:
                # Cập nhật trạng thái
                self.updateAccountStatus(profile_id, "Stopped")
                
                # Dọn dẹp
                if profile_id in self.bot_workers:
                    del self.bot_workers[profile_id]
                
        except Exception as e:
            print(f"Lỗi khi xử lý bot finished: {str(e)}")
    
    def onModeChanged(self, index):
        """Xử lý khi mode chính thay đổi"""
        try:
            # Cập nhật mode cho tất cả account
            for i in range(self.accounts_layout.count()):
                widget = self.accounts_layout.itemAt(i).widget()
                if isinstance(widget, QFrame) and widget.property("profile_id"):
                    profile_id = widget.property("profile_id")
                    combo = self.findChild(QComboBox, f"mode_combo_{profile_id}")
                    if combo:
                        combo.setCurrentIndex(index)
                        
        except Exception as e:
            print(f"Lỗi khi thay đổi mode: {str(e)}")
    
    def showAccountsPage(self):
        """Hiển thị trang Account Dashboard"""
        self.clearContentArea()
        self.set_active_menu_item("Accounts")
        dashboard_container = QWidget()
        dashboard_layout = QVBoxLayout(dashboard_container)
        title_label = QLabel("Accounts Dashboard")
        title_label.setFont(QFont("Arial", 24, QFont.Weight.Bold))
        dashboard_layout.addWidget(title_label)
        subtitle_label = QLabel("Manage your Twitter bot accounts and activities")
        subtitle_label.setFont(QFont("Arial", 14))
        subtitle_label.setStyleSheet("color: #757575")
        dashboard_layout.addWidget(subtitle_label)
        header_layout = QHBoxLayout()
        header_layout.addStretch()
        add_account_button = QPushButton("Add Account")
        add_account_button.setStyleSheet("""
            background-color: #1DA1F2;
            color: white;
            padding: 8px 15px;
            border-radius: 5px;
            font-weight: bold;
        """)
        add_account_button.setFixedWidth(150)
        add_account_button.clicked.connect(self.addNewAccount)
        header_layout.addWidget(add_account_button)
        dashboard_layout.addLayout(header_layout)
        dashboard_layout.addSpacing(20)
        # Container cho account cards
        self.accounts_container = QWidget()
        self.accounts_layout = QGridLayout(self.accounts_container)
        self.accounts_layout.setContentsMargins(0, 0, 0, 0)
        self.accounts_layout.setSpacing(20)
        # Scroll area cho accounts
        scroll_area = QScrollArea()
        scroll_area.setWidgetResizable(True)
        scroll_area.setFrameShape(QFrame.Shape.NoFrame)
        scroll_area.setHorizontalScrollBarPolicy(Qt.ScrollBarPolicy.ScrollBarAsNeeded)
        scroll_area.setWidget(self.accounts_container)
        dashboard_layout.addWidget(scroll_area)
        self.content_layout.addWidget(dashboard_container)
        self.loadAccounts()
    
    def set_active_menu_item(self, menu_name):
        """Set the active menu item in the sidebar"""
        # Reset all menu buttons to default style
        for name, button in self.sidebar_menu_buttons.items():
            button.setStyleSheet("""
                QPushButton {
                    text-align: left;
                    padding: 15px 20px;
                    border-radius: 0px;
                }
                QPushButton:hover {
                    background-color: #e8f5e9;
                }
            """)
        
        # Set the active menu button
        if menu_name in self.sidebar_menu_buttons:
            self.sidebar_menu_buttons[menu_name].setStyleSheet("""
                QPushButton {
                    text-align: left;
                    padding: 15px 20px;
                    border-radius: 0px;
                    background-color: #e8f5e9;
                    border-left: 5px solid #1DA1F2;
                }
                QPushButton:hover {
                    background-color: #e8f5e9;
                }
            """)
    
    def showSettingsPage(self):
        """Show the settings page"""
        self.clearContentArea()
        
        # Update the active menu item
        self.set_active_menu_item("Settings")
        
        # Create container for settings
        settings_container = QWidget()
        settings_layout = QVBoxLayout(settings_container)
        
        # Add title
        title_label = QLabel("Settings")
        title_label.setFont(QFont("Arial", 24, QFont.Weight.Bold))
        settings_layout.addWidget(title_label)
        
        subtitle_label = QLabel("Configure your Twitter Bot")
        subtitle_label.setStyleSheet("color: #757575")
        subtitle_label.setFont(QFont("Arial", 14))
        settings_layout.addWidget(subtitle_label)
        settings_layout.addSpacing(20)
        
        # Create settings sections
        settings_scroll = QScrollArea()
        settings_scroll.setWidgetResizable(True)
        settings_scroll.setFrameShape(QFrame.Shape.NoFrame)
        
        settings_scroll_content = QWidget()
        scroll_layout = QVBoxLayout(settings_scroll_content)
        
        # General Settings
        general_group = QGroupBox("General Settings")
        general_group.setStyleSheet("""
            QGroupBox {
                font-weight: bold;
                border: 1px solid #E1E8ED;
                border-radius: 8px;
                margin-top: 15px;
                padding-top: 15px;
            }
            QGroupBox::title {
                subcontrol-origin: margin;
                left: 10px;
                padding: 0 5px;
            }
        """)
        general_layout = QFormLayout(general_group)
        general_layout.setVerticalSpacing(15)
        
        # Theme setting
        theme_layout = QHBoxLayout()
        theme_label = QLabel("Theme:")
        theme_combo = QComboBox()
        theme_combo.addItems(["Light", "Dark", "System"])
        theme_combo.setCurrentText("Light")
        theme_layout.addWidget(theme_combo)
        theme_layout.addStretch()
        general_layout.addRow(theme_label, theme_combo)
        
        # Start on system boot
        start_boot = QCheckBox("Start on system boot")
        start_boot.setChecked(True)
        general_layout.addRow("", start_boot)
        
        # Notification setting
        enable_notifications = QCheckBox("Enable notifications")
        enable_notifications.setChecked(True)
        general_layout.addRow("", enable_notifications)
        
        scroll_layout.addWidget(general_group)
        
        # Bot Settings
        bot_group = QGroupBox("Bot Settings")
        bot_group.setStyleSheet("""
            QGroupBox {
                font-weight: bold;
                border: 1px solid #E1E8ED;
                border-radius: 8px;
                margin-top: 15px;
                padding-top: 15px;
            }
            QGroupBox::title {
                subcontrol-origin: margin;
                left: 10px;
                padding: 0 5px;
            }
        """)
        bot_layout = QFormLayout(bot_group)
        bot_layout.setVerticalSpacing(15)
        
        # Default post frequency
        frequency_layout = QHBoxLayout()
        frequency_spinbox = QSpinBox()
        frequency_spinbox.setRange(1, 24)
        frequency_spinbox.setValue(3)
        frequency_label = QLabel("hours")
        frequency_layout.addWidget(frequency_spinbox)
        frequency_layout.addWidget(frequency_label)
        frequency_layout.addStretch()
        bot_layout.addRow("Default posting frequency:", frequency_layout)
        
        # Auto-retry on API failure
        auto_retry = QCheckBox("Auto-retry on Twitter API failure")
        auto_retry.setChecked(True)
        bot_layout.addRow("", auto_retry)
        
        # Max retry attempts
        retry_layout = QHBoxLayout()
        retry_spinbox = QSpinBox()
        retry_spinbox.setRange(1, 10)
        retry_spinbox.setValue(3)
        retry_spinbox.setEnabled(auto_retry.isChecked())
        retry_label = QLabel("attempts")
        retry_layout.addWidget(retry_spinbox)
        retry_layout.addWidget(retry_label)
        retry_layout.addStretch()
        bot_layout.addRow("Max retry attempts:", retry_layout)
        
        # Connect auto-retry checkbox to spinbox enabled state
        auto_retry.stateChanged.connect(lambda state: retry_spinbox.setEnabled(state == Qt.CheckState.Checked.value))
        
        scroll_layout.addWidget(bot_group)
        
        # API Settings
        api_group = QGroupBox("API Settings")
        api_group.setStyleSheet("""
            QGroupBox {
                font-weight: bold;
                border: 1px solid #E1E8ED;
                border-radius: 8px;
                margin-top: 15px;
                padding-top: 15px;
            }
            QGroupBox::title {
                subcontrol-origin: margin;
                left: 10px;
                padding: 0 5px;
            }
        """)
        api_layout = QFormLayout(api_group)
        api_layout.setVerticalSpacing(15)
        
        # API Key
        api_key = QLineEdit()
        api_key.setEchoMode(QLineEdit.EchoMode.Password)
        api_key.setPlaceholderText("Enter your API key")
        api_key.setText("●●●●●●●●●●●●●●●●●●●●")
        show_api = QPushButton("Show")
        show_api.setFixedWidth(60)
        
        api_key_layout = QHBoxLayout()
        api_key_layout.addWidget(api_key)
        api_key_layout.addWidget(show_api)
        
        api_layout.addRow("Twitter API Key:", api_key_layout)
        
        # API Secret
        api_secret = QLineEdit()
        api_secret.setEchoMode(QLineEdit.EchoMode.Password)
        api_secret.setPlaceholderText("Enter your API secret")
        api_secret.setText("●●●●●●●●●●●●●●●●●●●●")
        show_secret = QPushButton("Show")
        show_secret.setFixedWidth(60)
        
        api_secret_layout = QHBoxLayout()
        api_secret_layout.addWidget(api_secret)
        api_secret_layout.addWidget(show_secret)
        
        api_layout.addRow("Twitter API Secret:", api_secret_layout)
        
        # Access Token
        access_token = QLineEdit()
        access_token.setEchoMode(QLineEdit.EchoMode.Password)
        access_token.setPlaceholderText("Enter your access token")
        access_token.setText("●●●●●●●●●●●●●●●●●●●●")
        show_token = QPushButton("Show")
        show_token.setFixedWidth(60)
        
        token_layout = QHBoxLayout()
        token_layout.addWidget(access_token)
        token_layout.addWidget(show_token)
        
        api_layout.addRow("Access Token:", token_layout)
        
        # Access Token Secret
        token_secret = QLineEdit()
        token_secret.setEchoMode(QLineEdit.EchoMode.Password)
        token_secret.setPlaceholderText("Enter your access token secret")
        token_secret.setText("●●●●●●●●●●●●●●●●●●●●")
        show_token_secret = QPushButton("Show")
        show_token_secret.setFixedWidth(60)
        
        token_secret_layout = QHBoxLayout()
        token_secret_layout.addWidget(token_secret)
        token_secret_layout.addWidget(show_token_secret)
        
        api_layout.addRow("Access Token Secret:", token_secret_layout)
        
        scroll_layout.addWidget(api_group)
        
        # Add stretch to push everything to the top
        scroll_layout.addStretch()
        
        settings_scroll.setWidget(settings_scroll_content)
        settings_layout.addWidget(settings_scroll)
        
        # Create save button
        save_btn_layout = QHBoxLayout()
        save_btn_layout.addStretch()
        
        save_btn = QPushButton("Save Settings")
        save_btn.setStyleSheet("""
            QPushButton {
                background-color: #1DA1F2;
                color: white;
                border-radius: 4px;
                padding: 8px 20px;
                font-weight: bold;
            }
            QPushButton:hover {
                background-color: #0d8ecf;
            }
        """)
        save_btn.setFixedWidth(150)
        save_btn_layout.addWidget(save_btn)
        
        settings_layout.addLayout(save_btn_layout)
        settings_layout.addSpacing(15)
        
        # Add the settings container to the content area
        self.content_layout.addWidget(settings_container)
    
    def showStatsPage(self):
        """Show the statistics page with processed tweet information"""
        self.clearContentArea()
        
        # Update the active menu item
        self.set_active_menu_item("Statistics")
        
        # Create container for stats
        stats_container = QWidget()
        stats_layout = QVBoxLayout(stats_container)
        
        # Add title
        title_label = QLabel("Processed Tweets Analytics")
        title_label.setFont(QFont("Arial", 24, QFont.Weight.Bold))
        stats_layout.addWidget(title_label)
        
        subtitle_label = QLabel("View statistics of tweets processed by your bots")
        subtitle_label.setStyleSheet("color: #757575")
        subtitle_label.setFont(QFont("Arial", 14))
        stats_layout.addWidget(subtitle_label)
        stats_layout.addSpacing(20)
        
        # Create time filter controls
        filter_layout = QHBoxLayout()
        
        account_label = QLabel("Account:")
        filter_layout.addWidget(account_label)
        
        # Get all accounts from database
        accounts = self.db.get_all_accounts()
        account_names = ["All Accounts"]
        if accounts:
            for account in accounts:
                account_names.append(f"@{account.get('username')}")
        
        account_combo = QComboBox()
        account_combo.addItems(account_names)
        filter_layout.addWidget(account_combo)
        
        filter_layout.addSpacing(20)
        
        time_label = QLabel("Time Range:")
        filter_layout.addWidget(time_label)
        
        time_combo = QComboBox()
        time_combo.addItems(["Last 24 Hours", "Last 7 Days", "Last 30 Days", "Last 90 Days", "Custom"])
        filter_layout.addWidget(time_combo)
        
        filter_layout.addStretch()
        
        refresh_btn = QPushButton("Refresh Data")
        refresh_btn.setStyleSheet("""
            QPushButton {
                background-color: #1DA1F2;
                color: white;
                border-radius: 4px;
                padding: 6px 12px;
            }
            QPushButton:hover {
                background-color: #0d8ecf;
            }
        """)
        refresh_btn.clicked.connect(self.refreshTweetStats)
        filter_layout.addWidget(refresh_btn)
        
        export_btn = QPushButton("Export Data")
        export_btn.setStyleSheet("""
            QPushButton {
                background-color: #1DA1F2;
                color: white;
                border-radius: 4px;
                padding: 6px 12px;
            }
            QPushButton:hover {
                background-color: #0d8ecf;
            }
        """)
        filter_layout.addWidget(export_btn)
        
        stats_layout.addLayout(filter_layout)
        stats_layout.addSpacing(20)
        
        # Add metrics cards based on actual tweet data
        metrics_grid = QGridLayout()
        metrics_grid.setSpacing(15)
        
        # Get tweet stats from database for all accounts
        total_tweets = 0
        total_likes = 0
        total_follows = 0
        total_retweets = 0
        engagement_rate = 0.0
        
        for account in accounts:
            profile_id = account.get('profile_id')
            stats = self.db.get_account_stats(profile_id)
            if stats:
                total_tweets += stats.get('replies_sent', 0)
                total_likes += stats.get('likes_given', 0)
                total_follows += stats.get('follows_made', 0)
                total_retweets += stats.get('retweets', 0)
        
        # Calculate engagement rate (simple example: likes + retweets / total tweets)
        if total_tweets > 0:
            engagement_rate = round(((total_likes + total_retweets) / total_tweets) * 100, 1)
        
        # Metrics cards data [title, value, icon]
        metrics_data = [
            ["Processed Tweets", str(total_tweets), "📝"],
            ["Engagement Rate", f"{engagement_rate}%", "📊"],
            ["Likes Given", str(total_likes), "❤️"],
            ["Retweets", str(total_retweets), "🔄"],
            ["Follows Made", str(total_follows), "👤"],
            ["Success Rate", f"{100.0 if total_tweets == 0 else round((total_tweets - 0) / total_tweets * 100, 1)}%", "✅"]
        ]
        
        for i, (title, value, icon) in enumerate(metrics_data):
            row, col = i // 3, i % 3
            
            card = QFrame()
            card.setStyleSheet("""
                QFrame {
                    background-color: white;
                    border-radius: 8px;
                    border: 1px solid #E1E8ED;
                }
            """)
            card.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Fixed)
            card.setMinimumHeight(120)
            
            card_layout = QVBoxLayout(card)
            
            # Icon and Title in same row
            header_layout = QHBoxLayout()
            icon_label = QLabel(icon)
            icon_label.setFont(QFont("Arial", 14))
            header_layout.addWidget(icon_label)
            
            title_label = QLabel(title)
            title_label.setStyleSheet("color: #657786; font-weight: normal;")
            header_layout.addWidget(title_label)
            header_layout.addStretch()
            
            card_layout.addLayout(header_layout)
            
            # Value
            value_label = QLabel(value)
            value_label.setFont(QFont("Arial", 24, QFont.Weight.Bold))
            card_layout.addWidget(value_label)
            
            metrics_grid.addWidget(card, row, col)
        
        stats_layout.addLayout(metrics_grid)
        stats_layout.addSpacing(20)
        
        # Add tweet processing table
        table_label = QLabel("Recent Processed Tweets")
        table_label.setFont(QFont("Arial", 18, QFont.Weight.Bold))
        stats_layout.addWidget(table_label)
        
        # Create table for processed tweets
        self.tweets_table = QTableWidget()
        self.tweets_table.setColumnCount(7)
        self.tweets_table.setHorizontalHeaderLabels([
            "Time", "Username", "Tweet Link", "Reply", "Status", "Actions", "Views"
        ])
        
        # Set table properties
        self.tweets_table.horizontalHeader().setSectionResizeMode(0, QHeaderView.ResizeMode.ResizeToContents)
        self.tweets_table.horizontalHeader().setSectionResizeMode(1, QHeaderView.ResizeMode.ResizeToContents)
        self.tweets_table.horizontalHeader().setSectionResizeMode(2, QHeaderView.ResizeMode.Stretch)
        self.tweets_table.horizontalHeader().setSectionResizeMode(3, QHeaderView.ResizeMode.Stretch)
        self.tweets_table.horizontalHeader().setSectionResizeMode(4, QHeaderView.ResizeMode.ResizeToContents)
        self.tweets_table.horizontalHeader().setSectionResizeMode(5, QHeaderView.ResizeMode.ResizeToContents)
        self.tweets_table.horizontalHeader().setSectionResizeMode(6, QHeaderView.ResizeMode.ResizeToContents)
        
        self.tweets_table.setAlternatingRowColors(True)
        self.tweets_table.setStyleSheet("""
            QTableWidget {
                gridline-color: #E1E8ED;
                border: 1px solid #E1E8ED;
                border-radius: 8px;
                background-color: white;
            }
            QHeaderView::section {
                background-color: #F5F8FA;
                padding: 6px;
                font-weight: bold;
                border: none;
                border-bottom: 1px solid #E1E8ED;
            }
            QTableWidget::item {
                padding: 6px;
            }
        """)
        
        # Load recent tweet data (up to 100 rows)
        self.loadRecentTweetData()
        
        stats_layout.addWidget(self.tweets_table)
        
        # Add the stats container to the content area
        self.content_layout.addWidget(stats_container)
        
    def refreshTweetStats(self):
        """Refresh the tweet statistics data"""
        # Reload the showStatsPage to refresh all data
        self.showStatsPage()
        
    def loadRecentTweetData(self):
        """Load recent tweet data into the tweets table"""
        try:
            if not hasattr(self, 'tweets_table') or self.tweets_table is None:
                return
            # Clear existing rows
            self.tweets_table.setRowCount(0)
            
            # Get tweet data from database using get_replied_tweets instead
            tweets = self.db.get_replied_tweets(limit=100)  # Get up to 100 recent tweets
            
            if not tweets:
                return
                
            # Add rows to the table
            for i, tweet in enumerate(tweets):
                self.tweets_table.insertRow(i)
                
                # Format data
                time_str = tweet.get('replied_at', '')
                username = tweet.get('username', '')
                tweet_url = f"https://twitter.com/i/web/status/{tweet.get('tweet_id', '')}"
                reply_text = tweet.get('reply_text', '')
                status = "Thành công"  # Since these are replied tweets, they were successful
                
                # Add items to the row
                self.tweets_table.setItem(i, 0, QTableWidgetItem(str(time_str)))
                self.tweets_table.setItem(i, 1, QTableWidgetItem(username))
                
                # Create clickable link for tweet URL
                url_item = QTableWidgetItem(tweet_url)
                url_item.setForeground(QBrush(QColor("#1DA1F2")))
                self.tweets_table.setItem(i, 2, url_item)
                
                # Reply text might be long, show truncated version
                reply_item = QTableWidgetItem(reply_text[:100] + "..." if len(reply_text) > 100 else reply_text)
                self.tweets_table.setItem(i, 3, reply_item)
                
                # Status with color
                status_item = QTableWidgetItem(status)
                status_item.setForeground(QBrush(QColor("#34C759")))
                self.tweets_table.setItem(i, 4, status_item)
                
                # Actions taken (simplified since we don't have this data)
                actions_item = QTableWidgetItem("❤️ 👤")  # Default actions
                self.tweets_table.setItem(i, 5, actions_item)
                
                # Views (not available in current data)
                views_item = QTableWidgetItem("0")
                self.tweets_table.setItem(i, 6, views_item)
            
        except Exception as e:
            self.showMessage("Error", f"Could not load tweet data: {str(e)}")
    
    def showLogPage(self):
        """Show the comprehensive program log page"""
        self.clearContentArea()
        
        # Update the active menu item
        self.set_active_menu_item("Log")
        
        # Create container for logs
        logs_container = QWidget()
        logs_layout = QVBoxLayout(logs_container)
        
        # Add title
        title_label = QLabel("Program Logs")
        title_label.setFont(QFont("Arial", 24, QFont.Weight.Bold))
        logs_layout.addWidget(title_label)
        
        subtitle_label = QLabel("View and monitor all program activities, bot operations, and system events")
        subtitle_label.setStyleSheet("color: #757575")
        subtitle_label.setFont(QFont("Arial", 14))
        logs_layout.addWidget(subtitle_label)
        logs_layout.addSpacing(20)
        
        # Create filter controls container
        filter_container = QFrame()
        filter_container.setObjectName("filter_container")
        filter_container.setStyleSheet("""
            QFrame#filter_container {
                background-color: white;
                border-radius: 8px;
                border: 1px solid #e1e8ed;
                padding: 15px;
            }
        """)
        filter_layout = QGridLayout(filter_container)
        
        # Module filter
        module_label = QLabel("Module:")
        module_combo = QComboBox()
        module_combo.addItems(["All Modules", "Bot", "System", "Database", "Account Manager", "Network"])
        module_combo.setObjectName("module_filter")
        filter_layout.addWidget(module_label, 0, 0)
        filter_layout.addWidget(module_combo, 0, 1)
        
        # Log level filter
        level_label = QLabel("Log Level:")
        level_combo = QComboBox()
        level_combo.addItems(["All Levels", "INFO", "WARNING", "ERROR", "DEBUG", "SUCCESS"])
        level_combo.setObjectName("level_filter")
        filter_layout.addWidget(level_label, 0, 2)
        filter_layout.addWidget(level_combo, 0, 3)
        
        # Account filter
        account_label = QLabel("Account:")
        account_combo = QComboBox()
        account_combo.addItems(["All Accounts"])
        # Add accounts from database
        accounts = self.db.get_all_accounts()
        if accounts:
            for account in accounts:
                account_combo.addItem(f"@{account.get('username')}")
        account_combo.setObjectName("account_filter")
        filter_layout.addWidget(account_label, 1, 0)
        filter_layout.addWidget(account_combo, 1, 1)
        
        # Date range filter
        date_label = QLabel("Date Range:")
        date_combo = QComboBox()
        date_combo.addItems(["Last Hour", "Last 24 Hours", "Last 7 Days", "Last 30 Days", "Custom Range"])
        date_combo.setObjectName("date_filter")
        filter_layout.addWidget(date_label, 1, 2)
        filter_layout.addWidget(date_combo, 1, 3)
        
        # Search box
        search_label = QLabel("Search:")
        search_input = QLineEdit()
        search_input.setPlaceholderText("Search in logs...")
        search_input.setObjectName("search_input")
        filter_layout.addWidget(search_label, 2, 0)
        filter_layout.addWidget(search_input, 2, 1, 1, 3)
        
        # Filter buttons
        filter_buttons_layout = QHBoxLayout()
        filter_buttons_layout.addStretch()
        
        clear_filters_btn = QPushButton("Clear Filters")
        clear_filters_btn.setObjectName("clear_filters_btn")
        clear_filters_btn.clicked.connect(self.clearLogFilters)
        
        apply_filters_btn = QPushButton("Apply Filters")
        apply_filters_btn.setObjectName("apply_filters_btn")
        apply_filters_btn.setStyleSheet("background-color: #1DA1F2; color: white;")
        apply_filters_btn.clicked.connect(self.applyLogFilters)
        
        filter_buttons_layout.addWidget(clear_filters_btn)
        filter_buttons_layout.addWidget(apply_filters_btn)
        filter_layout.addLayout(filter_buttons_layout, 3, 0, 1, 4)
        
        logs_layout.addWidget(filter_container)
        logs_layout.addSpacing(20)
        
        # Log table
        self.log_table = QTableWidget()
        self.log_table.setObjectName("log_table")
        self.log_table.setColumnCount(6)
        self.log_table.setHorizontalHeaderLabels(["Timestamp", "Level", "Module", "Account", "Message", "Details"])
        
        # Set table properties
        self.log_table.horizontalHeader().setSectionResizeMode(0, QHeaderView.ResizeMode.ResizeToContents)  # Timestamp
        self.log_table.horizontalHeader().setSectionResizeMode(1, QHeaderView.ResizeMode.ResizeToContents)  # Level
        self.log_table.horizontalHeader().setSectionResizeMode(2, QHeaderView.ResizeMode.ResizeToContents)  # Module
        self.log_table.horizontalHeader().setSectionResizeMode(3, QHeaderView.ResizeMode.ResizeToContents)  # Account
        self.log_table.horizontalHeader().setSectionResizeMode(4, QHeaderView.ResizeMode.Stretch)          # Message
        self.log_table.horizontalHeader().setSectionResizeMode(5, QHeaderView.ResizeMode.ResizeToContents)  # Details
        
        self.log_table.setAlternatingRowColors(True)
        self.log_table.setStyleSheet("""
            QTableWidget#log_table {
                background-color: white;
                border: 1px solid #e1e8ed;
                border-radius: 8px;
                gridline-color: #e1e8ed;
            }
            QTableWidget#log_table::item {
                padding: 8px;
                border-bottom: 1px solid #e1e8ed;
            }
            QHeaderView::section {
                background-color: #f5f8fa;
                padding: 8px;
                border: none;
                border-bottom: 1px solid #e1e8ed;
                font-weight: bold;
            }
        """)
        
        # Add some sample logs (replace with actual log loading)
        self.loadLogData()
        
        logs_layout.addWidget(self.log_table)
        
        # Bottom controls
        bottom_controls = QHBoxLayout()
        bottom_controls.addStretch()
        
        # Auto-refresh controls
        auto_refresh_label = QLabel("Auto-refresh:")
        auto_refresh_combo = QComboBox()
        auto_refresh_combo.addItems(["Off", "5 seconds", "10 seconds", "30 seconds", "1 minute"])
        auto_refresh_combo.setCurrentText("10 seconds")
        auto_refresh_combo.setObjectName("auto_refresh_combo")
        auto_refresh_combo.currentTextChanged.connect(self.onAutoRefreshChanged)
        
        # Action buttons
        clear_logs_btn = QPushButton("Clear Logs")
        clear_logs_btn.setObjectName("clear_logs_btn")
        clear_logs_btn.clicked.connect(self.clearLogs)
        
        export_logs_btn = QPushButton("Export Logs")
        export_logs_btn.setObjectName("export_logs_btn")
        export_logs_btn.setStyleSheet("background-color: #1DA1F2; color: white;")
        export_logs_btn.clicked.connect(self.exportLogs)
        
        bottom_controls.addWidget(auto_refresh_label)
        bottom_controls.addWidget(auto_refresh_combo)
        bottom_controls.addSpacing(20)
        bottom_controls.addWidget(clear_logs_btn)
        bottom_controls.addWidget(export_logs_btn)
        
        logs_layout.addLayout(bottom_controls)
        
        # Add the logs container to the content area
        self.content_layout.addWidget(logs_container)
        
        # Start auto-refresh timer
        self.startLogAutoRefresh()
    
    def loadLogData(self):
        """Load log data into the table"""
        try:
            if not hasattr(self, 'log_table') or self.log_table is None:
                return
            # Clear existing rows
            self.log_table.setRowCount(0)
            
            # Get logs from database (implement this method in DatabaseManager)
            logs = self.db.get_logs(limit=1000)  # Get last 1000 logs
            
            if not logs:
                return
                
            # Add rows to table
            for log in logs:
                row = self.log_table.rowCount()
                self.log_table.insertRow(row)
                
                # Format timestamp
                timestamp = log.get('timestamp', '')
                if isinstance(timestamp, str):
                    try:
                        timestamp = datetime.fromisoformat(timestamp)
                    except ValueError:
                        pass
                if isinstance(timestamp, datetime):
                    timestamp = timestamp.strftime('%Y-%m-%d %H:%M:%S')
                
                # Set row data with appropriate colors
                self.log_table.setItem(row, 0, QTableWidgetItem(str(timestamp)))
                
                # Level with color
                level_item = QTableWidgetItem(log.get('level', 'INFO'))
                level = log.get('level', 'INFO').upper()
                if level == 'ERROR':
                    level_item.setForeground(QBrush(QColor("#FF3B30")))
                elif level == 'WARNING':
                    level_item.setForeground(QBrush(QColor("#FF9500")))
                elif level == 'SUCCESS':
                    level_item.setForeground(QBrush(QColor("#4CD964")))
                elif level == 'DEBUG':
                    level_item.setForeground(QBrush(QColor("#8E8E93")))
                self.log_table.setItem(row, 1, level_item)
                
                # Module
                self.log_table.setItem(row, 2, QTableWidgetItem(log.get('module', '')))
                
                # Account
                account = log.get('account', '')
                if account:
                    account = f"@{account}"
                self.log_table.setItem(row, 3, QTableWidgetItem(account))
                
                # Message
                self.log_table.setItem(row, 4, QTableWidgetItem(log.get('message', '')))
                
                # Details (if any)
                details = log.get('details', '')
                if details:
                    try:
                        # Try to format JSON details
                        if isinstance(details, str):
                            details = json.loads(details)
                        details = json.dumps(details, indent=2)
                    except:
                        pass
                self.log_table.setItem(row, 5, QTableWidgetItem(str(details)))
            
            # Scroll to bottom
            self.log_table.scrollToBottom()
            
        except Exception as e:
            self.showMessage("Error", f"Could not load logs: {str(e)}")
    
    def clearLogFilters(self):
        """Clear all log filters"""
        try:
            # Reset all filter controls
            self.findChild(QComboBox, "module_filter").setCurrentIndex(0)
            self.findChild(QComboBox, "level_filter").setCurrentIndex(0)
            self.findChild(QComboBox, "account_filter").setCurrentIndex(0)
            self.findChild(QComboBox, "date_filter").setCurrentIndex(0)
            self.findChild(QLineEdit, "search_input").clear()
            
            # Reload logs
            self.loadLogData()
            
        except Exception as e:
            self.showMessage("Error", f"Could not clear filters: {str(e)}")
    
    def applyLogFilters(self):
        """Apply selected filters to logs"""
        try:
            # Get filter values
            module = self.findChild(QComboBox, "module_filter").currentText()
            level = self.findChild(QComboBox, "level_filter").currentText()
            account = self.findChild(QComboBox, "account_filter").currentText()
            date_range = self.findChild(QComboBox, "date_filter").currentText()
            search_text = self.findChild(QLineEdit, "search_input").text().lower()
            
            # Get logs with filters
            logs = self.db.get_filtered_logs(
                module=module if module != "All Modules" else None,
                level=level if level != "All Levels" else None,
                account=account if account != "All Accounts" else None,
                date_range=date_range,
                search_text=search_text if search_text else None
            )
            
            # Update table
            self.log_table.setRowCount(0)
            for log in logs:
                # Add log to table (similar to loadLogData)
                row = self.log_table.rowCount()
                self.log_table.insertRow(row)
                # ... (same as in loadLogData)
            
        except Exception as e:
            self.showMessage("Error", f"Could not apply filters: {str(e)}")
    
    def clearLogs(self):
        """Clear all logs"""
        try:
            reply = QMessageBox.question(
                self,
                "Clear Logs",
                "Are you sure you want to clear all logs? This action cannot be undone.",
                QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No,
                QMessageBox.StandardButton.No
            )
            
            if reply == QMessageBox.StandardButton.Yes:
                # Clear logs in database
                self.db.clear_logs()
                # Reload empty table
                self.loadLogData()
                
        except Exception as e:
            self.showMessage("Error", f"Could not clear logs: {str(e)}")
    
    def exportLogs(self):
        """Export logs to file"""
        try:
            # Get save file path
            file_path, _ = QFileDialog.getSaveFileName(
                self,
                "Export Logs",
                "",
                "CSV Files (*.csv);;JSON Files (*.json);;Text Files (*.txt)"
            )
            
            if not file_path:
                return
                
            # Get current filters
            module = self.findChild(QComboBox, "module_filter").currentText()
            level = self.findChild(QComboBox, "level_filter").currentText()
            account = self.findChild(QComboBox, "account_filter").currentText()
            date_range = self.findChild(QComboBox, "date_filter").currentText()
            
            # Export logs
            self.db.export_logs(
                file_path,
                module=module if module != "All Modules" else None,
                level=level if level != "All Levels" else None,
                account=account if account != "All Accounts" else None,
                date_range=date_range
            )
            
            self.showMessage("Success", "Logs exported successfully!")
            
        except Exception as e:
            self.showMessage("Error", f"Could not export logs: {str(e)}")
    
    def startLogAutoRefresh(self):
        """Start auto-refresh timer for logs"""
        try:
            # Get refresh interval
            interval_text = self.findChild(QComboBox, "auto_refresh_combo").currentText()
            if interval_text == "Off":
                return
                
            # Convert to milliseconds
            if interval_text == "5 seconds":
                interval = 5000
            elif interval_text == "10 seconds":
                interval = 10000
            elif interval_text == "30 seconds":
                interval = 30000
            elif interval_text == "1 minute":
                interval = 60000
            else:
                return
                
            # Create and start timer
            self.log_refresh_timer = QTimer()
            self.log_refresh_timer.timeout.connect(self.loadLogData)
            self.log_refresh_timer.start(interval)
            
        except Exception as e:
            print(f"Error starting log auto-refresh: {str(e)}")
    
    def onAutoRefreshChanged(self, value):
        """Handle auto-refresh interval change"""
        try:
            # Stop existing timer if any
            if hasattr(self, 'log_refresh_timer'):
                self.log_refresh_timer.stop()
            
            # Start new timer if not "Off"
            if value != "Off":
                self.startLogAutoRefresh()
                
        except Exception as e:
            print(f"Error changing auto-refresh: {str(e)}")

    def clearContentArea(self):
        """Clear all widgets from the content area layout and reset instance widgets/layouts"""
        while self.content_layout.count():
            item = self.content_layout.takeAt(0)
            widget = item.widget()
            if widget:
                widget.deleteLater()
        # Reset all instance widgets/layouts to None
        self.accounts_layout = None
        self.accounts_container = None
        self.log_table = None
        self.tweets_table = None

    def showMessage(self, title, message):
        """Show a message dialog"""
        QMessageBox.information(self, title, message)

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = TwitterBotGUI()
    window.show()
    sys.exit(app.exec())