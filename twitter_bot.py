import os
import time
import bs4
import asyncio
from database import DatabaseManager
from dotenv import load_dotenv
from selenium import webdriver
from selenium.webdriver.common.by import By
from selenium.webdriver.support.ui import WebDriverWait
from selenium.webdriver.support import expected_conditions as EC
from selenium.common.exceptions import NoSuchElementException
from datetime import datetime, timedelta
import google.generativeai as genai
from langdetect import detect as lang_detect
from PyQt6.QtCore import QObject, pyqtSignal, QThread
from PyQt6.QtWidgets import QMessageBox, QGroupBox, QLineEdit
from selenium.webdriver.chrome.options import Options
import openai
import sys
import functools
from typing import Dict
import requests
import random
from selenium.webdriver.chrome.service import Service  # Uncomment dòng này
from selenium.webdriver.chrome.options import Options
import sqlite3
from concurrent.futures import ThreadPoolExecutor
from functools import lru_cache
import queue
import threading
from urllib.parse import quote  # Thêm import này ở đầu file
import re
import json

# Load environment variables
load_dotenv()

# Constants
class TwitterConstants:
    # Status
    STATUS_SUCCESS = "Thành công"
    STATUS_FAILED = "Thất bại"
    
    # Selectors
    SELECTOR_TWEET = 'article[data-testid="tweet"]'
    SELECTOR_TWEET_TEXT = '[data-testid="tweetText"]'
    SELECTOR_REPLY_BUTTON = '[data-testid="reply"]'
    SELECTOR_TWEET_BUTTON = '[data-testid="tweetButton"]'
    SELECTOR_TWEET_TEXTAREA = '[data-testid="tweetTextarea_0"]'
    SELECTOR_FOLLOW_BUTTON = '[data-testid$="-follow"], [data-testid$="-unfollow"]'
    SELECTOR_LIKE_BUTTON = '[data-testid="like"]'
    SELECTOR_USER_NAME = '[data-testid="User-Name"]'
    SELECTOR_VERIFIED_ICON = '[data-testid="icon-verified"]'
    SELECTOR_VERIFIED_BADGE = '[aria-label*="Verified"]'

# Cache decorator for API responses
def cache_api_response(ttl_seconds=300):
    cache = {}
    def decorator(func):
        @functools.wraps(func)
        def wrapper(*args, **kwargs):
            key = str(args) + str(kwargs)
            now = time.time()
            if key in cache:
                result, timestamp = cache[key]
                if now - timestamp < ttl_seconds:
                    return result
            result = func(*args, **kwargs)
            cache[key] = (result, now)
            return result
        return wrapper
    return decorator

class DatabasePool:
    _instance = None
    _lock = threading.Lock()
    
    def __new__(cls):
        with cls._lock:
            if cls._instance is None:
                cls._instance = super().__new__(cls)
                cls._instance.init_pool()
            return cls._instance
            
    def init_pool(self):
        self.pool = queue.Queue()
        for _ in range(10):  # Tạo sẵn 10 kết nối
            conn = sqlite3.connect('settings.db', check_same_thread=False)
            self.pool.put(conn)
            
    def get_connection(self):
        return self.pool.get()
        
    def return_connection(self, conn):
        self.pool.put(conn)

class TwitterBot(QObject):
    # Khai báo signals trước các phương thức
    tweet_processed_signal = pyqtSignal(str, str, str, str, str, bool, bool, bool, int, int, str, str)
    log_signal = pyqtSignal(str)
    status_signal = pyqtSignal(str)
    error_signal = pyqtSignal(str)
    stop_signal = pyqtSignal()

    def __init__(self, credentials, main_window=None):
        super().__init__()
        # Initialize signals first
        self._initialize_signals()
        
        # Initialize database connection in main thread
        self.db = DatabaseManager(main_window=main_window)
        self.main_window = main_window
        
        # Basic attributes
        self.is_paused = False
        self.should_run = True
        self.force_stop = False
        self.username = credentials.get('username', '')
        self.profile_id = credentials.get('profile_id', '')
        
        # Thêm mode_id attribute
        self.mode_id = 1  # Mặc định là feed mode
        
        if not self.profile_id:
            self.error_signal.emit("Không tìm thấy profile_id!")
            raise ValueError("Không tìm thấy profile_id")

        # Load settings from database in main thread
        self.load_initial_settings()
        
        # Initialize API settings
        self.use_gemini = credentials.get('use_gemini', True)
        self.gemini_key = credentials.get('gemini_key', '')
        self.chatgpt_key = credentials.get('chatgpt_key', '')

        # Initialize caches
        self.api_cache = {}
        self.api_cache_ttl = 300  # 5 minutes
        self.tweet_cache = {}
        self.replied_tweets = self.load_replied_tweets()

        # Initialize thread pool
        self.thread_pool = ThreadPoolExecutor(max_workers=3)

        # Khởi tạo các thuộc tính khác...
        self.user_mode = credentials.get('user_mode', False)
        self.target_users = credentials.get('target_users', [])
        self.replied_users = set()

    def load_initial_settings(self):
        """Load initial settings from database in main thread"""
        try:
            settings = self.db.get_settings(self.profile_id)
            if settings:
                # Convert database values to appropriate types
                self.filter_settings = {
                    'max_replies': int(settings.get('max_replies', 50)),
                    'min_views': int(settings.get('min_views', 0)),
                    'skip_replies': bool(int(settings.get('skip_replies', 1))),
                    'skip_retweets': bool(int(settings.get('skip_retweets', 1))),
                    'skip_japanese': bool(int(settings.get('skip_japanese', 0))),
                    'auto_like': bool(int(settings.get('auto_like', 1))),
                    'auto_follow_verified': bool(int(settings.get('auto_follow_verified', 0))),
                    'auto_retweet': bool(int(settings.get('auto_retweet', 0))),
                    'japanese_only': bool(int(settings.get('japanese_only', 0))),
                    'reply_first_only': bool(int(settings.get('reply_first_only', 0))),
                    'minimize_window': bool(int(settings.get('minimize_window', 0))),
                    'time_limit_hours': int(settings.get('time_limit_hours', 24)),
                    'time_limit_minutes': int(settings.get('time_limit_minutes', 0)),
                    'interval': int(settings.get('interval', 30)),
                    'reply_interval': int(settings.get('reply_interval', 0))
                }

                # Update instance variables from settings
                self.time_limit_hours = self.filter_settings['time_limit_hours']
                self.time_limit_minutes = self.filter_settings['time_limit_minutes']
                self.interval = self.filter_settings['interval']
                self.reply_interval = self.filter_settings['reply_interval']
                self.should_minimize = self.filter_settings['minimize_window']

                # Schedule settings
                self.schedule_enabled = bool(int(settings.get('schedule_enabled', 0)))
                self.start_time = settings.get('start_time', '09:00')
                self.end_time = settings.get('end_time', '17:00')
                self.schedule_days = settings.get('schedule_days', '0,1,2,3,4,5,6').split(',')

                self.log(f"Đã tải cấu hình từ database cho {self.profile_id}")

        except Exception as e:
            self.log(f"Lỗi khi đọc cấu hình cho {self.profile_id}: {str(e)}")
            # Use default settings
            self.filter_settings = {
                'max_replies': 50,
                'min_views': 0,
                'skip_replies': True,
                'skip_retweets': True,
                'skip_japanese': False,
                'auto_like': True,
                'auto_follow_verified': False,
                'auto_retweet': False,
                'japanese_only': False,
                'reply_first_only': False,
                'minimize_window': False,
                'time_limit_hours': 24,
                'time_limit_minutes': 0,
                'interval': 30,
                'reply_interval': 0
            }
            # Set default instance variables
            self.time_limit_hours = 24
            self.time_limit_minutes = 0
            self.interval = 30
            self.reply_interval = 0
            self.should_minimize = False
            self.schedule_enabled = False
            self.start_time = '09:00'
            self.end_time = '17:00'
            self.schedule_days = ['0','1','2','3','4','5','6']

    def update_settings_from_db(self):
        """Update settings from database in the current thread"""
        db_pool = DatabasePool()
        conn = None
        try:
            conn = db_pool.get_connection()
            cursor = conn.cursor()
            
            cursor.execute("SELECT * FROM settings WHERE profile_id = ?", (self.profile_id,))
            settings = cursor.fetchone()
            
            if settings:
                self.filter_settings.update({
                    'max_replies': int(settings.get('max_replies', 50)),
                    'min_views': int(settings.get('min_views', 0)),
                    'skip_replies': bool(int(settings.get('skip_replies', 1))),
                    'skip_retweets': bool(int(settings.get('skip_retweets', 1))),
                    'skip_japanese': bool(int(settings.get('skip_japanese', 0))),
                    'auto_like': bool(int(settings.get('auto_like', 1))),
                    'auto_follow_verified': bool(int(settings.get('auto_follow_verified', 0))),
                    'auto_retweet': bool(int(settings.get('auto_retweet', 0))),
                    'japanese_only': bool(int(settings.get('japanese_only', 0))),
                    'reply_first_only': bool(int(settings.get('reply_first_only', 0))),
                    'minimize_window': bool(int(settings.get('minimize_window', 0))),
                    'time_limit_hours': int(settings.get('time_limit_hours', 24)),
                    'time_limit_minutes': int(settings.get('time_limit_minutes', 0)),
                    'interval': int(settings.get('interval', 30)),
                    'reply_interval': int(settings.get('reply_interval', 0))
                })
                self.log(f"Đã cập nhật cấu hình từ database")
                return True
                
        except Exception as e:
            self.log(f"Lỗi khi cập nhật cấu hình từ database: {str(e)}")
            return False
            
        finally:
            if conn:
                db_pool.return_connection(conn)

    def _initialize_signals(self):
        """Initialize all signals"""
        try:
            # Connect basic signals
            self.log_signal.connect(self.log_message)
            self.error_signal.connect(self.error_message)
        except Exception as e:
            print(f"Error initializing signals: {str(e)}")

    def log_message(self, message):
        """Default log message handler"""
        print(f"LOG: {message}")
        # Lưu log vào database nếu có self.db
        db = getattr(self, 'db', None)
        if not db and hasattr(self, 'main_window') and hasattr(self.main_window, 'db'):
            db = self.main_window.db
        if db:
            db.add_log(level="INFO", module="TwitterBot", account=getattr(self, 'username', None), message=message)

    def error_message(self, message):
        """Default error message handler"""
        print(f"ERROR: {message}")
        db = getattr(self, 'db', None)
        if not db and hasattr(self, 'main_window') and hasattr(self.main_window, 'db'):
            db = self.main_window.db
        if db:
            db.add_log(level="ERROR", module="TwitterBot", account=getattr(self, 'username', None), message=message)

    def log(self, message):
        """Ghi log với signal và lưu vào database"""
        if hasattr(self, 'log_signal'):
            clean_message = message.replace(f"[@{getattr(self, 'username', '')}]", "").strip()
            self.log_signal.emit(clean_message)
            print(clean_message)
            db = getattr(self, 'db', None)
            if not db and hasattr(self, 'main_window') and hasattr(self.main_window, 'db'):
                db = self.main_window.db
            if db:
                db.add_log(level="INFO", module="TwitterBot", account=getattr(self, 'username', None), message=clean_message)

    def error_log(self, message):
        """Ghi log lỗi với signal và lưu vào database"""
        if hasattr(self, 'error_signal'):
            clean_message = message.replace(f"[@{getattr(self, 'username', '')}]", "").strip()
            error_message = f"ERROR: {clean_message}"
            self.error_signal.emit(error_message)
            print(error_message)
            db = getattr(self, 'db', None)
            if not db and hasattr(self, 'main_window') and hasattr(self.main_window, 'db'):
                db = self.main_window.db
            if db:
                db.add_log(level="ERROR", module="TwitterBot", account=getattr(self, 'username', None), message=clean_message)

    def check_logged_in(self):
        """Kiểm tra xem đã đăng nhập thành công chưa"""
        try:
            if self.is_paused:  # Kiểm tra pause
                return False
            # Kiểm tra xem đã có driver chưa
            if not hasattr(self, 'driver'):
                self.log("Chưa kết nối được với trình duyệt")
                return False
                
            # Kiểm tra xem đã có driver chưa
            if not hasattr(self, 'driver'):
                self.log("Chưa kết nối được với trình duyệt")
                return False
                
            # Kiểm tra URL hiện tại
            current_url = self.driver.current_url
            
            # Nếu chưa ở trang Twitter thì navigate đến
            if "twitter.com" not in current_url:
                self.driver.get("https://twitter.com/home")
                time.sleep(10)  # Tăng thời gian chờ từ 3s lên 10s
                
            # Thêm thời gian chờ để trang load hoàn toàn
            time.sleep(5)
            
            # Kiểm tra các element để xác nhận đã login
            for _ in range(3):  # Thử kiểm tra 3 lần
                try:
                    # Kiểm tra sự hiện diện của các element chỉ có khi đã đăng nhập
                    is_logged_in = any([
                        len(self.driver.find_elements(By.CSS_SELECTOR, '[data-testid="SideNav_NewTweet_Button"]')) > 0,
                        len(self.driver.find_elements(By.CSS_SELECTOR, '[data-testid="AppTabBar_Profile_Link"]')) > 0,
                        len(self.driver.find_elements(By.CSS_SELECTOR, '[aria-label="Twitter"]')) > 0
                    ])
                    
                    if is_logged_in:
                        self.log("Đã đăng nhập thành công")
                        return True
                        
                except Exception:
                    pass
                    
                time.sleep(5)  # Đợi thêm 5s trước khi thử lại
                
            self.log("Chưa đăng nhập")
            return False
                
        except Exception as e:
            self.log(f"Lỗi khi kiểm tra đăng nhập: {str(e)}")
            return False

    def setup_driver(self):
        """Setup Chrome driver với GPMLogin remote debugging"""
        try:
            # Gọi API GPMLogin để mở profile
            url = f"http://127.0.0.1:19995/api/v3/profiles/start/{self.profile_id}"
            response = requests.get(url)
            
            if response.status_code != 200:
                raise Exception(f"Không thể mở profile {self.profile_id}: {response.text}")
            
            data = response.json()
            debug_port = data.get('debug_port')
            
            if not debug_port:
                raise Exception(f"Không nhận được debug port từ GPMLogin")
            
            # Cấu hình remote debugging
            options = Options()
            options.debugger_address = f"127.0.0.1:{debug_port}"
            
            # Kết nối tới browser đã mở
            self.driver = webdriver.Chrome(options=options)
            
            # Set window size nếu cần
            if self.should_minimize:
                self.driver.set_window_size(800, 600)
            
            self.log(f"Đã kết nối tới profile {self.profile_id} qua port {debug_port}")
            return True
        
        except Exception as e:
            self.error_log(f"Lỗi khi setup driver: {str(e)}")
            return False

    def cleanup(self):
        """Cleanup resources"""
        try:
            if hasattr(self, 'driver') and self.driver:
                self.driver.quit()
                self.driver = None
        except Exception as e:
            self.log(f"Error in cleanup: {str(e)}")

    def check_connection(self):
        """Kiểm tra kết nối remote debugging còn hoạt động không"""
        try:
            if not self.driver:
                return False
                
            # Thử truy cập URL hiện tại
            current_url = self.driver.current_url
            return True
            
        except Exception as e:
            self.log(f"Mất kết nối remote debugging: {str(e)}")
            return False

    def ensure_connection(func):
        """Decorator đảm bảo kết nối trước khi thực hiện thao tác"""
        def wrapper(self, *args, **kwargs):
            try:
                if not self.check_connection():
                    self.log("Phát hiện mất kết nối, đang thử kết nối lại...")
                    if not self.setup_driver():
                        raise Exception("Không thể kết nối lại remote debugging")
                return func(self, *args, **kwargs)
            except Exception as e:
                self.log(f"Lỗi khi thực hiện {func.__name__}: {str(e)}")
                return None
        return wrapper

    def minimize_window(self):
        """Di chuyển cửa sổ ra khỏi màn hình"""
        try:
            if self.driver:
                # Thay vì di chuyển cửa sổ ra xa, chỉ thu nhỏ xuống taskbar
                self.driver.minimize_window()
                time.sleep(0.5)
        except Exception as e:
            self.log(f"Error minimizing window: {str(e)}")
            
    def login(self):
        """Kiểm tra đăng nhập"""
        try:
            self.log("Đang kiểm tra đăng nhập...")
            login_status = self.check_logged_in()
            if login_status:
                self.log("Xác nhận đã đăng nhập thành công") 
                return True
            else:
                self.log("Chưa đăng nhập, vui lòng kiểm tra lại")
                return False
        except Exception as e:
            self.log(f"Lỗi khi kiểm tra đăng nhập: {str(e)}")
            return False

    def start(self):
        """Khởi động bot"""
        try:
            # Kết nối với trình duyệt
            if not self.setup_driver():
                self.error_signal.emit("Không thể khởi tạo driver cho profile " + self.profile_id)
                return False
            
            # Kiểm tra đăng nhập 
            if not self.login():
                self.error_signal.emit("Chưa đăng nhập Twitter!")
                return False

            # Nếu đã đăng nhập thành công
            self.log("Đã đăng nhập thành công, bắt đầu xử lý...")
            
            # Áp dụng minimize nếu được cấu hình
            if self.should_minimize:
                self.minimize_window()
            
            # Bắt đầu monitor feed và tiếp tục chạy
            self.monitor_feed()
            return True
            
        except Exception as e:
            self.error_signal.emit(f"Lỗi khi khởi động bot: {str(e)}")
            return False

    def get_recent_tweets(self, max_tweets=50):
        """Get the most recent tweets from the feed"""
        try:
            # Không cần cuộn lên đầu nữa
            self.scroll_feed(scroll_count=10)  # Giới hạn 10 lần cuộn
            
            tweets = self.wait.until(
                EC.presence_of_all_elements_located((By.CSS_SELECTOR, 'article[data-testid="tweet"]'))
            )
            
            if not tweets:
                return []

            self.log(f"Đã tìm thấy {len(tweets)} tweets tổng cộng")
            
            # Lọc tweets theo điều kiện 
            valid_tweets = []
            for tweet in tweets:
                try:
                    if self.should_process_tweet(tweet):
                        valid_tweets.append(tweet)
                except Exception as e:
                    self.log(f"Lỗi khi xử lý tweet: {str(e)}")
                    continue
                
            if not valid_tweets:
                self.log("Không tìm thấy tweet phù hợp, tiếp tục tìm kiếm...")
                return []  # Return empty list để tiếp tục tìm kiếm
            
            self.log(f"Có {len(valid_tweets)} tweets thỏa điều kiện")
            return valid_tweets[:max_tweets]
            
        except Exception as e:
            self.log(f"Lỗi khi lấy tweets gần đây: {str(e)}")
            return []  # Return empty list để tiếp tục tìm kiếm

    def wait_for_page_load(self):
        """Đợi cho trang tải hoàn tất"""
        try:
            # Đợi tối đa 30 giây
            return self.wait.until(
                EC.presence_of_element_located((By.CSS_SELECTOR, 'article[data-testid="tweet"]'))
            )
        except Exception as e:
            self.log(f"Lỗi khi đợi trang tải: {str(e)}")
            return False
    def resume(self):
        """Tiếp tục hoạt động của bot"""
        try:
            # Đọc lại cấu hình từ database trong thread hiện tại
            try:
                if self.main_window:
                    settings = self.main_window.db.get_settings(self.profile_id)
                else:
                    settings = self.db.get_settings(self.profile_id)
                    
                if settings:
                    self.filter_settings.update({
                        'max_replies': int(settings.get('max_replies', 50)),
                        'min_views': int(settings.get('min_views', 0)),
                        'skip_replies': bool(int(settings.get('skip_replies', 1))),
                        'skip_retweets': bool(int(settings.get('skip_retweets', 1))),
                        'skip_japanese': bool(int(settings.get('skip_japanese', 0))),
                        'auto_like': bool(int(settings.get('auto_like', 1))),
                        'auto_follow_verified': bool(int(settings.get('auto_follow_verified', 0))),
                        'auto_retweet': bool(int(settings.get('auto_retweet', 0))),
                        'japanese_only': bool(int(settings.get('japanese_only', 0))),
                        'reply_first_only': bool(int(settings.get('reply_first_only', 0))),
                        'minimize_window': bool(int(settings.get('minimize_window', 0))),
                        'time_limit_hours': int(settings.get('time_limit_hours', 24)),
                        'time_limit_minutes': int(settings.get('time_limit_minutes', 0)),
                        'interval': int(settings.get('interval', 30)),
                        'reply_interval': int(settings.get('reply_interval', 0))
                    })
                    self.log(f"Đã cập nhật cấu hình từ database khi resume")
                else:
                    self.log("Không tìm thấy cấu hình trong database khi resume!")
                    return False
            except Exception as e:
                self.log(f"Lỗi khi đọc cấu hình từ database trong resume: {str(e)}")
                return False

            self.is_paused = False
            self.log("Đang tiếp tục hoạt động...")
            
            # Kiểm tra đăng nhập trước
            if not self.check_logged_in():
                if not self.login():
                    self.log("Không thể đăng nhập!")
                    return False
            
            # Tiếp tục monitor feed
            self.monitor_feed(skip_login=True)
            return True
                
        except Exception as e:
            self.log(f"Lỗi khi tiếp tục bot: {str(e)}")
            return False
    def monitor_feed(self, interval=10):
        """Monitor Twitter feed"""
        try:
            # Chỉ kiểm tra đăng nhập lần đầu tiên
            if not hasattr(self, '_initial_login_checked'):
                if not self.check_logged_in():
                    return False
                self._initial_login_checked = True
                
            if self.is_paused:
                return False            
                
            # Xác định URL dựa trên mode
            if self.mode_id == 1:  # Feed mode
                self.driver.get("https://twitter.com/home")
            elif self.mode_id == 2:  # User mode
                if not self.target_users:
                    self.log("Chưa cấu hình user cần theo dõi")
                    return False
                self.driver.get(f"https://twitter.com/{self.target_users[0]}")
            elif self.mode_id == 3:  # Comments mode
                return self.reply_to_commenters_mode(interval)
            elif self.mode_id == 4:  # Trending mode
                # Kiểm tra thời gian chờ trước khi tìm kiếm
                if hasattr(self, 'main_window') and self.main_window:
                    db = self.main_window.db
                else:
                    db = self.db
                    
                try:
                    # Lấy cấu hình mới nhất từ database
                    settings = db.get_settings(self.profile_id)
                    if settings:
                        reply_interval = int(settings.get('reply_interval', 60))  # Mặc định 60s nếu không có
                    else:
                        reply_interval = 60  # Giá trị mặc định
                        
                    # Kiểm tra thời gian của bài đăng gần nhất
                    cursor = db.execute_query("""
                        SELECT posted_at FROM posted_tweets 
                        WHERE profile_id = ? 
                        ORDER BY posted_at DESC LIMIT 1
                    """, (self.profile_id,))
                    last_post = cursor.fetchone()
                    
                    if last_post:
                        last_post_time = datetime.strptime(last_post[0], '%Y-%m-%d %H:%M:%S')
                        time_since_last_post = (datetime.now() - last_post_time).total_seconds()
                        
                        if time_since_last_post < reply_interval:
                            wait_time = reply_interval - time_since_last_post
                            self.log(f"Cần đợi thêm {wait_time:.1f} giây trước khi tìm bài mới")
                            time.sleep(wait_time)
                            self.log("Đã hết thời gian đợi, bắt đầu tìm bài mới...")
                            return True  # Return True để không kiểm tra đăng nhập lại
                except Exception as e:
                    self.log(f"Lỗi khi kiểm tra thời gian chờ: {str(e)}")
                
                # Lấy keyword và kiểm tra
                keyword = self.get_keyword()
                if not keyword:
                    self.log("Chưa nhập từ khóa tìm kiếm")
                    return False
                    
                # Encode keyword để hỗ trợ tiếng Nhật và các ký tự đặc biệt
                encoded_keyword = quote(keyword)
                
                # Thêm # nếu keyword không bắt đầu bằng #
                if not keyword.startswith('#'):
                    encoded_keyword = '%23' + encoded_keyword
                    
                search_url = f"https://x.com/search?q={encoded_keyword}&src=trend_click&vertical=trends"
                self.log(f"Tìm kiếm URL: {search_url}")
                self.driver.get(search_url)
                
            time.sleep(3)
            
            processed_any = False  # Flag để kiểm tra xem đã xử lý tweet nào chưa
            processed_tweets = set()  # Set để lưu ID của các tweets đã xử lý
            
            # Scroll và xử lý tweets
            while True:
                if self.is_paused or self.force_stop:
                    break

                # Cuộn trước để load thêm tweets mới
                self.scroll_feed(scroll_count=10)
                    
                # Tìm và xử lý tweets mới
                tweets = self.driver.find_elements(By.CSS_SELECTOR, "article[data-testid='tweet']")
                
                for tweet in tweets:
                    try:
                        # Kiểm tra xem tweet đã được xử lý chưa
                        tweet_id = self.get_tweet_id(tweet)
                        if tweet_id in processed_tweets:
                            continue
                            
                        if self.mode_id == 4:  # Trending mode
                            if self.should_process_tweet(tweet):
                                result = self.process_trending_tweet(tweet)
                                if result:
                                    processed_any = True
                                    processed_tweets.add(tweet_id)
                                    # Đợi reply_interval rồi tiếp tục
                                    try:
                                        settings = self.db.get_settings(self.profile_id)
                                        if settings:
                                            reply_interval = int(settings.get('reply_interval', 60))
                                            if reply_interval > 0:
                                                self.log(f"Đợi {reply_interval} giây...")
                                                time.sleep(reply_interval)
                                    except Exception as e:
                                        self.log(f"Lỗi khi lấy reply_interval: {str(e)}")
                        else:
                            if self.should_process_tweet(tweet):
                                self.process_tweet(tweet)
                                processed_any = True
                                processed_tweets.add(tweet_id)
                    except Exception as e:
                        self.log(f"Lỗi khi xử lý tweet: {str(e)}")
                        continue  # Bỏ qua tweet lỗi và tiếp tục với tweet tiếp theo
                
                if not processed_any and self.mode_id == 4:
                    self.log("Không tìm thấy tweet phù hợp, tiếp tục tìm kiếm...")
                    return True  # Trả về True để tiếp tục tìm kiếm trong lần chạy tiếp theo
                
                if self.is_paused or self.force_stop:
                    break
                    
            return True
            
        except Exception as e:
            self.log(f"Lỗi khi monitor feed: {str(e)}")
            return True  # Return True để tiếp tục chạy

    def process_trending_tweet(self, tweet_element):
        """Xử lý tweet trong mode trending"""
        try:
            # Lấy database connection
            if hasattr(self, 'main_window') and self.main_window:
                db = self.main_window.db
            else:
                db = self.db
                
            try:
                # Lấy cấu hình mới nhất từ database
                settings = db.get_settings(self.profile_id)
                if settings:
                    reply_interval = int(settings.get('reply_interval', 60))  # Mặc định 60s nếu không có
                else:
                    reply_interval = 60  # Giá trị mặc định
                    
                # Kiểm tra thời gian của bài đăng gần nhất
                cursor = db.execute_query("""
                    SELECT posted_at FROM posted_tweets 
                    WHERE profile_id = ? 
                    ORDER BY posted_at DESC LIMIT 1
                """, (self.profile_id,))
                last_post = cursor.fetchone()
                
                if last_post:
                    last_post_time = datetime.strptime(last_post[0], '%Y-%m-%d %H:%M:%S')
                    time_since_last_post = (datetime.now() - last_post_time).total_seconds()
                    
                    if time_since_last_post < reply_interval:
                        wait_time = reply_interval - time_since_last_post
                        self.log(f"Cần đợi thêm {wait_time:.1f} giây trước khi tìm bài mới")
                        time.sleep(wait_time)
                        self.log("Đã hết thời gian đợi, bắt đầu tìm bài mới...")
                        return False  # Return False để bỏ qua tweet này và tìm tweet khác sau khi đợi
            except Exception as e:
                self.log(f"Lỗi khi kiểm tra thời gian chờ: {str(e)}")

            # Kiểm tra các điều kiện lọc tweet trước
            if not self.should_process_tweet(tweet_element):
                self.log("Bỏ qua tweet này và tiếp tục tìm tweet khác...")
                return False
                
            # Lấy tweet ID để kiểm tra trùng lặp
            tweet_id = self.get_tweet_id(tweet_element)
            if not tweet_id:
                self.log("Không lấy được tweet ID, bỏ qua")
                return False
                
            # Kiểm tra xem tweet đã được xử lý chưa
            cursor = db.execute_query(
                "SELECT COUNT(*) FROM posted_tweets WHERE profile_id = ? AND original_id = ?",
                (self.profile_id, tweet_id)
            )
            processed = cursor.fetchone()[0]
            
            if processed > 0:
                self.log(f"Tweet {tweet_id} đã được xử lý trước đó, bỏ qua")
                return False

            # Lấy nội dung tweet
            text_element = tweet_element.find_element(By.CSS_SELECTOR, "[data-testid='tweetText']")
            content = text_element.text
            
            self.log(f"Đang xử lý tweet {tweet_id}: {content[:100]}...")
            
            # Khởi tạo biến lưu danh sách file media đã tải
            downloaded_media = []
            
            # Kiểm tra và tải media (video và ảnh)
            media_urls = self.get_media_urls(tweet_element)
            if media_urls['video'] or media_urls['images']:
                self.log(f"Phát hiện media trong tweet: {len(media_urls['images'])} ảnh, video: {'Có' if media_urls['video'] else 'Không'}")
                # Tạo thư mục downloaded_media nếu chưa có
                media_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), "downloaded_media")
                downloaded_files = self.download_media(media_urls, media_dir)
                if downloaded_files:
                    self.log(f"Đã tải {len(downloaded_files)} file media thành công: {downloaded_files}")
                    downloaded_media = downloaded_files
                else:
                    self.log("Không thể tải media")

            # Viết lại nội dung bằng AI
            if self.use_gemini:
                rewritten = self.rewrite_with_gemini(content)
            else:
                rewritten = self.rewrite_with_chatgpt(content)
                
            if rewritten and rewritten != content:  # Kiểm tra nội dung có thay đổi không
                # Kiểm tra nội dung trùng lặp trong database
                cursor = db.execute_query(
                    "SELECT COUNT(*) FROM posted_tweets WHERE profile_id = ? AND posted_content = ?",
                    (self.profile_id, rewritten)
                )
                duplicate = cursor.fetchone()[0]
                
                if duplicate > 0:
                    self.log("Nội dung viết lại trùng với bài đã đăng trước đó, bỏ qua")
                    return False
                    
                # Đăng tweet mới kèm media nếu có
                if self.post_tweet(rewritten, downloaded_media if downloaded_media else None):
                    try:
                        # Lưu tweet đã xử lý vào database
                        cursor = db.execute_query(
                            """INSERT INTO posted_tweets 
                               (profile_id, original_id, original_content, posted_content, posted_at) 
                               VALUES (?, ?, ?, ?, ?)""",
                            (self.profile_id, tweet_id, content, rewritten, 
                             datetime.now().strftime('%Y-%m-%d %H:%M:%S'))
                        )
                        
                        # Commit thay đổi vào database
                        if hasattr(cursor, 'connection'):
                            cursor.connection.commit()
                        elif hasattr(db, 'conn'):
                            db.conn.commit()
                        elif hasattr(db, 'commit'):
                            db.commit()
                        
                        self.log(f"Đã xử lý và đăng lại tweet thành công")
                        
                        # Chờ reply_interval giây sau khi post thành công
                        if reply_interval > 0:
                            self.log(f"Đã post thành công, đợi {reply_interval} giây trước khi tiếp tục...")
                            time.sleep(reply_interval)
                            self.log("Đã hết thời gian đợi, tiếp tục tìm bài mới...")
                        
                        return True
                            
                    except Exception as db_error:
                        self.log(f"Lỗi khi lưu vào database: {str(db_error)}")
                        return True  # Vẫn trả về True vì tweet đã được đăng thành công
            else:
                self.log("Bỏ qua tweet do nội dung không thay đổi sau khi viết lại và tiếp tục")
                return False

        except Exception as e:
            self.log(f"Lỗi khi xử lý tweet trong mode trending: {str(e)}")
            return False

    def get_keyword(self):
        """Lấy keyword từ UI"""
        try:
            # Tìm keyword input trong widget của tài khoản
            for i in range(self.main_window.accounts_layout.count()):
                widget = self.main_window.accounts_layout.itemAt(i).widget()
                if isinstance(widget, QGroupBox) and widget.property("profile_id") == self.profile_id:
                    keyword_input = widget.findChild(QLineEdit, f"keyword_input_{self.profile_id}")
                    if keyword_input:
                        return keyword_input.text().strip()
            return None
        except Exception as e:
            self.log(f"Lỗi khi lấy keyword: {str(e)}")
            return None

    def cleanup(self):
        """Dọn dẹp tài nguyên"""
        try:
            if self.driver:
                # KHÔNG quit() driver khi dùng remote debugging
                self.driver = None
                self.wait = None

            if hasattr(self, 'model'):
                del self.model
                
            # Lưu danh sách tweet đã reply trước khi dừng
            self.save_replied_tweets()
            
            if hasattr(self, 'thread_pool'):
                self.thread_pool.shutdown()
            if hasattr(self, 'driver'):
                self.driver = None
            if hasattr(self, 'db_pool'):
                while not self.db_pool.pool.empty():
                    conn = self.db_pool.get_connection()
                    conn.close()
            
        except Exception as e:
            self.log(f"Lỗi khi cleanup: {str(e)}")

    def get_tweet_id(self, tweet_element):
        """Extract unique identifier for a tweet"""
        try:
            # Save information immediately to avoid stale element
            links = []
            time_href = None
            
            try:
                # Method 1: Get from status link
                links = tweet_element.find_elements(By.TAG_NAME, "a")
                links = [link.get_attribute("href") for link in links if link.get_attribute("href")]
            except:
                pass
            
            try:
                # Method 2: Get from time element
                time_element = tweet_element.find_element(By.CSS_SELECTOR, 'time')
                time_href = time_element.find_element(By.XPATH, '..').get_attribute('href')
            except:
                pass

            # Process saved links
            if links:
                for href in links:
                    if href and "/status/" in href:
                        return href
                
            if time_href:
                return time_href

            return None

        except Exception as e:
            self.log(f"Error getting tweet ID: {str(e)}")
            return None

    def get_tweet_timestamp(self, tweet_element):
        """Extract timestamp from a tweet"""
        try:
            # Thử nhiều cách để tìm time element
            time_element = None
            
            # Cách 1: Tìm trực tiếp thẻ time
            try:
                time_element = tweet_element.find_element(By.CSS_SELECTOR, 'time')
            except:
                pass
                
            # Cách 2: Tìm trong thẻ a chứa timestamp    
            if not time_element:
                try:
                    time_element = tweet_element.find_element(By.CSS_SELECTOR, 'a[href*="/status/"] time')
                except:
                    pass
                    
            # Cách 3: Tìm theo data-testid
            if not time_element:
                try:
                    time_element = tweet_element.find_element(By.CSS_SELECTOR, '[data-testid="timestamp"]')
                except:
                    pass
                    
            if not time_element:
                self.log("Không tìm thấy time element trong tweet")
                return None
                
            relative_time = time_element.text.strip().lower()
            current_time = datetime.now()
            
            self.log(f"Thời gian hiển thị trên tweet: {relative_time}")
            
            # Xử lý định dạng thời gian tương đối
            if any(c.isdigit() for c in relative_time):
                number = int(''.join(filter(str.isdigit, relative_time)))
                
                if 'giây' in relative_time or 's' in relative_time:
                    tweet_time = current_time - timedelta(seconds=number)
                elif 'phút' in relative_time or 'm' in relative_time:
                    tweet_time = current_time - timedelta(minutes=number)
                elif 'giờ' in relative_time or 'h' in relative_time:
                    tweet_time = current_time - timedelta(hours=number)
                elif 'ngày' in relative_time or 'd' in relative_time:
                    tweet_time = current_time - timedelta(days=number)
                else:
                    # Xử lý định dạng "month day" nếu không phải thời gian tương đối
                    months = {
                        'jan': 1, 'feb': 2, 'mar': 3, 'apr': 4, 'may': 5, 'jun': 6,
                        'jul': 7, 'aug': 8, 'sep': 9, 'oct': 10, 'nov': 11, 'dec': 12,
                        'january': 1, 'february': 2, 'march': 3, 'april': 4, 'june': 6,
                        'july': 7, 'august': 8, 'september': 9, 'october': 10, 'november': 11, 'december': 12
                    }
                    
                    parts = relative_time.split()
                    if len(parts) == 2 or parts[0] in months:
                        month = months[parts[0]]
                        day = int(parts[1])
                        year = current_time.year
                        if month > current_time.month:
                            year -= 1
                        tweet_time = datetime(year, month, day)
                    else:
                        self.log(f"Không nhận dạng được định dạng thời gian: {relative_time}")
                        return None
                    
                self.log(f"Đã tính được thời gian tweet: {tweet_time}")
                return tweet_time

            return None

        except Exception as e:
            self.log(f"Lỗi khi tìm time element: {str(e)}")
            return None

    def is_own_tweet(self, tweet_element):
        """Check if the tweet is from our own account"""
        try:
            # Tìm username trong tweet
            username_element = tweet_element.find_element(By.CSS_SELECTOR, '[data-testid="User-Name"]')
            username_links = username_element.find_elements(By.TAG_NAME, "a")
            
            # Lấy username từ link profile (format: @username)
            for link in username_links:
                href = link.get_attribute("href")
                if href:
                    tweet_username = href.split("/")[-1].lower()
                    our_username = self.username.lower() if self.username else ""
                    
                    # So sánh username (bỏ @ nếu có)
                    tweet_username = tweet_username.lstrip("@")
                    our_username = our_username.lstrip("@")
                    
                    if tweet_username == our_username:
                        self.log(f"Phát hiện tweet của mình (@{tweet_username})")
                        return True
            
            return False
            
        except Exception as e:
            self.log(f"Lỗi kiểm tra tweet ownership: {str(e)}")
            return False

    def is_reply_tweet(self, tweet_element):
        """Check if the tweet is a reply"""
        try:
            reply_indicators = tweet_element.find_elements(By.CSS_SELECTOR, '[data-testid="socialContext"]')
            if reply_indicators:
                return True

            replying_to = tweet_element.find_elements(By.XPATH, './/*[contains(text(), "Replying to")]')
            if replying_to:
                return True

            return False
        except Exception as e:
            self.log(f"Error checking if tweet is reply: {str(e)}")
            return True

    def clean_text(self, text):
        """Clean text content"""
        # Remove URLs
        text = re.sub(r'http[s]?://(?:[a-zA-Z]|[0-9]|[$-_@.&+]|[!*\\(\\),]|(?:%[0-9a-fA-F][0-9a-fA-F]))+', '', text)
        # Remove mentions
        text = re.sub(r'@\w+', '', text)
        # Remove extra whitespace
        text = ' '.join(text.split())
        return text.strip()

    def clean_hashtags(self, text):
        """Clean and deduplicate hashtags"""
        # Tách nội dung chính và hashtags
        main_content = text.split('#')[0].strip()
        hashtag_part = '#' + '#'.join(text.split('#')[1:]) if '#' in text else ''
        
        if hashtag_part:
            # Lấy danh sách hashtags duy nhất
            hashtags = re.findall(r'#\w+', hashtag_part)
            unique_hashtags = list(dict.fromkeys(hashtags))  # Loại bỏ trùng lặp
            
            # Giới hạn số lượng hashtag (ví dụ: tối đa 5 hashtag)
            if len(unique_hashtags) > 5:
                unique_hashtags = unique_hashtags[:5]
            
            # Ghép nội dung chính với hashtags đã xử lý
            return f"{main_content} {' '.join(unique_hashtags)}"
        
        return main_content

    @cache_api_response(ttl_seconds=300)
    def generate_ai_response(self, tweet_text: str) -> str:
        """Generate AI response with caching"""
        try:
            if not tweet_text:
                self.log("Tweet text trống")
                return "Hmm"
            
            try:
                language = lang_detect(tweet_text)
                self.log(f"Ngôn ngữ phát hiện: {language}")
            except Exception as e:
                self.log(f"Lỗi phát hiện ngôn ngữ: {str(e)}")
                language = 'en'

            # Tạo prompt dựa trên ngôn ngữ phát hiện được
            prompts = {
                'ja': f"""
                このツイートに対して短い返信を日本語で作成てください:
                ツイート: "{tweet_text}"
                条件:
                - 100文字以内
                - 共感的で友好的な内容
                - ハッシュタグや絵文字なし
                - 簡潔で分かりやすい日本語
                """,
                'vi': f"""
                Tạo câu trả lời ngắn gọn bằng tiếng Việt cho tweet này:
                Tweet: "{tweet_text}"
                Yêu cầu:
                - Dưới 100 ký tự
                - Thể hiện sự đồng cảm và thân thiện
                - Không dùng hashtag hoặc emoji
                - Ngôn ngữ đơn giản, dễ hiểu
                """,
                'ko': f"""
                이 트윗에 대한 짧은 한국어 답글을 작성해주세요:
                트윗: "{tweet_text}"
                조건:
                - 100자 이내
                - 공감적이고 친근한 내용
                - 해시태그나 이모지 사용 금지
                - 간단하고 이해하기 쉬운 한국어
                """,
                'zh': f"""
                请用中文为这条推文创建简短回复：
                推文："{tweet_text}"
                要求：
                - 100字以内
                - 表达同理心和友好
                - 不使用话题标签或表情符号
                - 使用简单易懂的中文
                """,
                'th': f"""
                สร้างการตอบกลับสั้นๆ เป็นภาษาไทยสำหรับทวีตนี้:
                ทวีต: "{tweet_text}"
                ข้อกำหนด:
                - ไม่เกิน 100 ตัวอักษร
                - แสดงความเห็นอกเห็นใจและเป็นมิตร
                - ไม่ใช้แฮชแท็กหรืออิโมจิ
                - ใช้ภาษาไทยที่เข้าใจง่าย
                """
            }

            # Default English prompt nếu không có prompt cho ngôn ngữ phát hiện được
            default_prompt = f"""
            Create a short response in the same language as the tweet:
            Tweet: "{tweet_text}"
            Requirements:
            - Under 100 characters
            - Show empathy and friendliness
            - No hashtags or emojis
            - Simple, clear language in the detected language ({language})
            - Response must start with a space
            """

            prompt = prompts.get(language, default_prompt)

            # Generate response using selected AI model
            if self.use_gemini:
                # Sử dụng Gemini API
                import google.generativeai as genai
                genai.configure(api_key=self.gemini_key)
                model = genai.GenerativeModel('gemini-1.5-pro')
                response = model.generate_content(
                    prompt,
                    generation_config={
                        'temperature': 0.7,
                        'max_output_tokens': 100,
                    }
                )
                if response and hasattr(response, 'text'):
                    ai_reply = response.text.strip().strip('"')
                    
            else:
                # Sử dụng ChatGPT API
                from openai import OpenAI
                client = OpenAI(api_key=self.chatgpt_key)
                response = client.chat.completions.create(
                    model="gpt-3.5-turbo",
                    messages=[
                        {"role": "system", "content": f"You are a friendly Twitter user who responds in {language}."},
                        {"role": "user", "content": prompt}
                    ],
                    max_tokens=100,
                    temperature=0.7
                )
                ai_reply = response.choices[0].message.content.strip().strip('"')

            # Post-process response
            if not ai_reply.startswith(' '):
                ai_reply = ' ' + ai_reply
            
            ai_reply = self.clean_text(ai_reply)
            
            if len(ai_reply) > 100:
                while len(ai_reply.encode('utf-8')) > 97:
                    ai_reply = ai_reply[:-1]
                ai_reply += "..."
            
            return ai_reply if ai_reply.strip() else "Hmm"
            
        except Exception as e:
            self.log(f"Lỗi khi tạo phản hồi AI: {str(e)}")
            return "Hmm"

    def find_element_with_retry(self, by, value, max_attempts=3, check_interactable=False):
        """Find an element with retry logic for stale elements"""
        for attempt in range(max_attempts):
            try:
                if check_interactable:
                    element = self.wait.until(
                        EC.element_to_be_clickable((by, value))
                    )
                else:
                    element = self.wait.until(
                        EC.presence_of_element_located((by, value))
                    )
                    if not element.is_displayed():
                        raise Exception("Element is not visible")
                
                self.driver.execute_script("arguments[0].scrollIntoView(true);", element)
                time.sleep(1)
                
                return element
            except Exception as e:
                if attempt == max_attempts - 1:
                    print(f"Failed to find element after {max_attempts} attempts: {str(e)}")
                    raise e
                print(f"Retrying to find element... Attempt {attempt + 2}/{max_attempts}")
                time.sleep(1)
        return None
    def like_tweet(self, tweet_element):
        """Like a tweet before replying"""
        try:
            # Tìm nút like hoặc unlike với nhiều cách khác nhau
            like_button = None
            selectors = [
                '[aria-label*="Like"]',
                '[aria-label*="Liked"]', 
                '[data-testid="like"]',
                '[data-testid="unlike"]',
                '[role="button"][data-testid="like"]',
                '[role="button"][data-testid="unlike"]'
            ]
    
            for selector in selectors:
                try:
                    like_button = tweet_element.find_element(By.CSS_SELECTOR, selector)
                    if like_button and like_button.is_displayed():
                        break
                except:
                    continue
    
            if not like_button:
                self.log("Không tìm thấy nút like/unlike")
                return False
            
            # Kiểm tra xem đã like chưa
            if 'Liked' in like_button.get_attribute('aria-label'):
                self.log("Tweet đã được like trước đó")
                return True
                
            # Cuộn đến vị trí nút like
            self.driver.execute_script("arguments[0].scrollIntoView({block: 'center'});", like_button)
            time.sleep(1)
                
            # Thử click bằng JavaScript nếu click thông thường không được
            try:
                self.driver.execute_script("arguments[0].click();", like_button)                
            except Exception:
                like_button.click()
                
            time.sleep(2)
                
            # Xác nhận like đã thành công
            try:
                # Check if any of the 3 selectors for liked state exists
                liked_selectors = [
                    '[aria-label*="Liked"]',
                    '[data-testid="unlike"]', 
                    '[aria-label*="liked"]'
                ]
                
                for selector in liked_selectors:
                    try:
                        unlike_button = tweet_element.find_element(By.CSS_SELECTOR, selector)
                        if unlike_button:
                            self.log("Đã like tweet thành công")
                            return True
                    except NoSuchElementException:
                        continue
                        
                self.log("Không thể xác nhận like thành công")
                return False
                
            except Exception as e:
                self.log(f"Lỗi khi kiểm tra trạng thái like: {str(e)}")
                return False
    
        except Exception as e:
            self.log(f"Lỗi khi like tweet: {str(e)}")
            return False
    def check_verified_and_follow(self, username, profile_link, current_url):
        """Kiểm tra verified và follow user trong tab hiện tại"""
        if self.is_paused:  # Kiểm tra pause
            return False
        
        # Kiểm tra cấu hình auto_follow_verified
        if not self.filter_settings.get('auto_follow_verified', False):
            self.log(f"Bỏ qua follow @{username} do cấu hình auto_follow_verified = False")
            return False
            
            
        # Đọc lại cấu hình từ database để đảm bảo dùng giá trị mới nhất
        try:
            settings = self.db.get_settings(self.profile_id)
            if settings and not settings.get('auto_follow_verified', False):
                self.log(f"Bỏ qua follow @{username} do cấu hình trong database auto_follow_verified = False")
                return False
        except Exception as e:
            self.log(f"Lỗi khi đọc cấu hình follow từ database: {str(e)}")
            return False
        
        follow_success = False
        
        try:
            # Navigate to profile in current tab
            self.driver.get(profile_link)
            time.sleep(3)
            
            # Đợi trang profile load xong
            try:
                self.wait.until(EC.presence_of_element_located((By.CSS_SELECTOR, '[data-testid="primaryColumn"]')))
                time.sleep(2)
            except:
                self.log("Không thể load trang profile")
                return False

            # Kiểm tra verified badge 
            try:
                # Kiểm tra verified với nhiều selector
                verified = False
                verified_selectors = [
                    '[data-testid="UserVerifiedBadge"]',
                    '[data-testid="icon-verified"]',
                    '[aria-label*="Verified"]',
                    'svg[aria-label*="Verified"]'
                ]
                
                for selector in verified_selectors:
                    try:
                        verified_badge = self.driver.find_element(By.CSS_SELECTOR, selector)
                        if verified_badge and verified_badge.is_displayed():
                            verified = True
                            break
                    except:
                        continue

                if verified:
                    self.log(f"User @{username} có tick xanh")
                    
                    # Kiểm tra lại lần nữa trước khi follow
                    if not self.filter_settings.get('auto_follow_verified', False):
                        self.log(f"Bỏ qua follow @{username} do cấu hình auto_follow_verified = False")
                        return False
                    
                    # Nếu có tick xanh thì tìm và click nút follow
                    try:
                        follow_selectors = [
                            '[data-testid="followButton"]',
                            '[data-testid="follow"]',
                            '[data-testid$="-follow"]',
                            '[data-testid$="-unfollow"]'
                        ]
                        
                        follow_button = None
                        for selector in follow_selectors:
                            try:
                                buttons = self.driver.find_elements(By.CSS_SELECTOR, selector)
                                for button in buttons:
                                    if button.is_displayed():
                                        button_text = button.text.lower()
                                        if 'unfollow' not in button_text:
                                            follow_button = button
                                            break
                            except:
                                continue
                            if follow_button:
                                break

                        if follow_button:
                            # Cuộn đến nút follow
                            self.driver.execute_script("arguments[0].scrollIntoView({block: 'center'});", follow_button)
                            time.sleep(1)
                            
                            # Thử click thường trước
                            try:
                                self.driver.execute_script("arguments[0].click();", follow_button)
                            except:
                                # Nếu không được thì dùng JavaScript click
                                follow_button.click()
                            
                            time.sleep(2)
                            self.log(f"Đã follow user @{username}")
                            follow_success = True
                        else:
                            self.log(f"Đã following @{username} trước đó")
                            follow_success = True
                            
                    except Exception as e:
                        self.log(f"Không thể tìm hoặc click nút follow: {str(e)}")
                        
                else:
                    self.log(f"User @{username} không có tick xanh")
                
            except NoSuchElementException:
                self.log(f"User @{username} không có tick xanh")
                
        except Exception as e:
            self.log(f"Lỗi khi kiểm tra và follow user: {str(e)}")
            
        finally:
            # Navigate back to home page
            self.driver.get("https://twitter.com/home")
            time.sleep(random.uniform(2, 4))
            
        return follow_success

    def ensure_window_ready(self):
        """Ensure browser window is ready for interaction"""
        try:
            if self.driver:
                # Restore window if minimized
                if self.driver.get_window_rect()['width'] <= 0 or self.driver.get_window_rect()['height'] <= 0:
                    self.driver.maximize_window()
                    time.sleep(0.5)
                    self.driver.set_window_rect(x=-32000, y=-32000, width=800, height=600)
                
                # Wait for any animations to complete
                time.sleep(0.5)
                return True
        except Exception as e:
            self.log(f"Error preparing window: {str(e)}")
            return False

    def interact_with_element(self, element, interaction_type="click", text=None):
        """Safely interact with elements ensuring they are visible"""
        try:
            if not element:
                return False
                
            # Ensure window is ready
            self.ensure_window_ready()
            
            # Scroll element into center view
            self.driver.execute_script(
                "arguments[0].scrollIntoView({block: 'center', inline: 'center'});", 
                element
            )
            time.sleep(0.5)
            
            # Wait for element to be clickable
            element = self.wait.until(
                EC.element_to_be_clickable(element)
            )
            
            if interaction_type == "click":
                # Try regular click first
                try:
                    self.driver.execute_script("arguments[0].click();", element)                    

                except:
                    # Fall back to JavaScript click
                    element.click()
            elif interaction_type == "send_keys" or text is not None:
                element.clear()
                element.send_keys(text)
                
            time.sleep(0.5)
            return True
            
        except Exception as e:
            self.log(f"Error interacting with element: {str(e)}")
            return False

    def reply_to_tweet(self, tweet_element):
        """Modified reply function with reliable element interaction"""
        reply_success = False
        
        # Initialize statistics tracking variables
        stats = {
            "tweet_id": None,
            "username": None,
            "content": "",
            "response_time_ms": 0,
            "character_count": 0,
            "is_verified": False,
            "has_media": False,
            "interaction_count": 0
        }
        start_time = time.time()
        
        try:
            # Đọc lại cấu hình từ database trước khi xử lý
            try:
                settings = self.db.get_settings(self.profile_id)
                if settings:
                    self.filter_settings.update({
                        'auto_like': bool(int(settings.get('auto_like', 1))),
                        'auto_follow_verified': bool(int(settings.get('auto_follow_verified', 0))),
                        'auto_retweet': bool(int(settings.get('auto_retweet', 0)))
                    })
            except Exception as e:
                self.log(f"Lỗi khi đọc cấu hình từ database trong reply_to_tweet: {str(e)}")
                
            # Lấy tweet ID ngay từ đầu
            time_element = tweet_element.find_element(By.CSS_SELECTOR, 'time')
            tweet_link = time_element.find_element(By.XPATH, '..').get_attribute('href')
            tweet_id = tweet_link.split('/status/')[-1].split('?')[0]
            stats["tweet_id"] = tweet_id
            
            # Kiểm tra xem đã reply chưa
            if tweet_id in self.replied_tweets:
                self.log(f"Tweet {tweet_id} đã được reply trước đó, bỏ qua")
                return False
            
            # Lấy và lưu tất cả thông tin cần thiết NGAY TỪ ĐẦU
            username = None
            profile_link = None
            tweet_text = ""
            content = ""  # Nội dung gốc của tweet
            try:
                # Lấy username và profile link
                user_name_element = tweet_element.find_element(By.CSS_SELECTOR, '[data-testid="User-Name"]')
                profile_link_element = user_name_element.find_elements(By.TAG_NAME, 'a')[0]
                profile_link = profile_link_element.get_attribute('href')
                username = profile_link.split('/')[-1]
                stats["username"] = username
                
                # Kiểm tra xem user có tick xanh không
                try:
                    verified_badge = user_name_element.find_element(By.CSS_SELECTOR, '[data-testid="icon-verified"], [aria-label*="Verified"]')
                    stats["is_verified"] = True
                except:
                    stats["is_verified"] = False
                
                # Lấy nội dung tweet
                tweet_text_element = tweet_element.find_element(By.CSS_SELECTOR, '[data-testid="tweetText"]')
                tweet_text = tweet_text_element.text
                content = tweet_text
                stats["content"] = content
                stats["character_count"] = len(content)
                
                # Kiểm tra xem tweet có media không
                try:
                    media_elements = tweet_element.find_elements(By.CSS_SELECTOR, '[data-testid="tweetPhoto"], [data-testid="videoPlayer"]')
                    stats["has_media"] = len(media_elements) > 0
                except:
                    stats["has_media"] = False
                
                # Đếm số lượng tương tác (likes, retweets, etc)
                try:
                    interaction_elements = tweet_element.find_elements(By.CSS_SELECTOR, '[data-testid="reply"], [data-testid="like"], [data-testid="retweet"]')
                    stats["interaction_count"] = len(interaction_elements)
                except:
                    stats["interaction_count"] = 0
                
            except Exception as e:
                self.log(f"Lỗi khi lấy thông tin tweet: {str(e)}")
                return False  # Không thể lấy thông tin cần thiết, bỏ qua tweet này

            # Chờ một khoảng thời gian ngẫu nhiên trước khi reply
            time.sleep(random.uniform(2, 5))
            
            # Tương tác với nút reply
            reply_button = self.find_element_with_retry(By.CSS_SELECTOR, '[data-testid="reply"]', max_attempts=5)
            if reply_button:
                try:
                    # Cuộn đến nút reply
                    self.driver.execute_script("arguments[0].scrollIntoView({block: 'center'});", reply_button)
                    time.sleep(1)
                    
                    # Click nút reply
                    reply_button.click()
                    time.sleep(2)
                except Exception as e:
                    self.log(f"Lỗi khi click nút reply: {str(e)}")
                    return False
            else:
                self.log("Không tìm thấy nút reply")
                return False

            # Tương tác với textarea để nhập nội dung reply
            textarea = self.find_element_with_retry(By.CSS_SELECTOR, '[data-testid="tweetTextarea_0"]', max_attempts=5)
            if textarea:
                try:
                    # Nhập nội dung reply
                    textarea.clear()
                    
                    # Sử dụng nội dung gốc của tweet để trả lời
                    original_content = content
                    
                    # Thêm hashtag nếu có
                    if self.filter_settings.get('auto_hashtag', False):
                        hashtag = self.extract_hashtag(original_content)
                        if hashtag:
                            original_content += f" {hashtag}"
                    
                    # Viết lại nội dung bằng AI nếu được cấu hình
                    if self.use_gemini:
                        rewritten_content = self.rewrite_with_gemini(original_content)
                    else:
                        rewritten_content = self.rewrite_with_chatgpt(original_content)
                    
                    # Nếu có nội dung viết lại, sử dụng nó
                    final_content = ""
                    if rewritten_content and rewritten_content != original_content:
                        self.log(f"Đã viết lại nội dung tweet thành công")
                        textarea.send_keys(rewritten_content)
                        final_content = rewritten_content
                    else:
                        self.log("Nội dung không thay đổi hoặc không thể viết lại")
                        textarea.send_keys(original_content)
                        final_content = original_content
                    
                    time.sleep(1)
                except Exception as e:
                    self.log(f"Lỗi khi nhập nội dung reply: {str(e)}")
                    return False
            else:
                self.log("Không tìm thấy textarea để nhập nội dung reply")
                return False

            # Tương tác với nút gửi (tweet button)
            tweet_button = self.find_element_with_retry(By.CSS_SELECTOR, '[data-testid="tweetButton"]', max_attempts=5)
            if tweet_button:
                try:
                    # Cuộn đến nút gửi
                    self.driver.execute_script("arguments[0].scrollIntoView({block: 'center'});", tweet_button)
                    time.sleep(1)
                    
                    # Click nút gửi
                    tweet_button.click()
                    time.sleep(3)
                    
                    self.log(f"Đã gửi reply cho tweet {tweet_id}")
                    reply_success = True
                    
                    # Lưu lại tweet đã reply vào danh sách
                    self.replied_tweets.add(tweet_id)
                    self.save_replied_tweets()
                    
                    # Tính thời gian phản hồi
                    end_time = time.time()
                    stats["response_time_ms"] = int((end_time - start_time) * 1000)
                    
                except Exception as e:
                    self.log(f"Lỗi khi click nút gửi: {str(e)}")
                    return False
            else:
                self.log("Không tìm thấy nút gửi để đăng reply")
                return False

            # Quay lại trang chủ sau khi reply xong
            self.driver.get("https://twitter.com/home")
            time.sleep(2)
            
            # Kiểm tra và thực hiện like tweet nếu được cấu hình
            like_success = False
            if self.filter_settings.get('auto_like', False):
                self.log("Thực hiện like tweet sau khi reply...")
                like_success = self.like_tweet(tweet_element)
            
            # Kiểm tra và thực hiện follow user nếu được cấu hình
            follow_success = False
            current_url = self.driver.current_url
            if self.filter_settings.get('auto_follow_verified', False) and stats["is_verified"]:
                self.log("Thực hiện follow user sau khi reply...")
                follow_success = self.check_verified_and_follow(username, profile_link, current_url)
            
            # Phát signal với đầy đủ thông tin thống kê
            if hasattr(self, 'tweet_processed_signal') and reply_success:
                self.log("Phát signal với thông tin tweet đã xử lý...")
                # Định dạng timestamp hiện tại
                timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
                
                # Chuẩn bị dữ liệu thống kê bổ sung dưới dạng JSON
                additional_stats = {
                    "tweet_id": stats["tweet_id"],
                    "response_time_ms": stats["response_time_ms"],
                    "character_count": stats["character_count"],
                    "is_verified": stats["is_verified"],
                    "has_media": stats["has_media"],
                    "interaction_count": stats["interaction_count"]
                }
                
                # Convert to JSON string
                additional_stats_json = json.dumps(additional_stats)
                
                # Emit signal với đầy đủ thông tin
                self.tweet_processed_signal.emit(
                    self.profile_id,        # profile_id
                    stats["username"],      # username
                    stats["content"],       # original_content
                    final_content,          # response_content
                    timestamp,              # timestamp
                    reply_success,          # reply_success
                    like_success,           # like_success
                    follow_success,         # follow_success
                    stats["response_time_ms"], # response_time
                    stats["character_count"],  # character_count
                    additional_stats_json,   # additional_stats
                    current_url              # current_url
                )
                
            return reply_success

        except Exception as e:
            self.log(f"Lỗi khi reply tweet: {str(e)}")
            
            # Phát signal với thông tin lỗi nếu có
            if hasattr(self, 'tweet_processed_signal'):
                timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
                error_stats = {
                    "tweet_id": stats.get("tweet_id", "unknown"),
                    "error": str(e)
                }
                error_stats_json = json.dumps(error_stats)
                self.tweet_processed_signal.emit(
                    self.profile_id,
                    stats.get("username", "unknown"),
                    stats.get("content", ""),
                    "",
                    timestamp,
                    False,  # reply_success
                    False,  # like_success
                    False,  # follow_success
                    stats.get("response_time_ms", 0),
                    stats.get("character_count", 0),
                    error_stats_json,
                    self.driver.current_url if hasattr(self, 'driver') else ""
                )
            
            return False

    def load_replied_tweets(self):
        """Load danh sách tweet đã reply từ database hoặc file (nếu có)"""
        try:
            # Nếu bạn lưu trong database:
            if hasattr(self, 'db'):
                rows = self.db.execute_query(
                    "SELECT tweet_id FROM replied_tweets WHERE profile_id = ?", (self.profile_id,)
                ).fetchall()
                return set(row[0] for row in rows)
            # Nếu bạn lưu trong file:
            # with open(f"replied_{self.profile_id}.json", "r", encoding="utf-8") as f:
            #     return set(json.load(f))
        except Exception as e:
            self.log(f"Lỗi khi load replied_tweets: {str(e)}")
            return set()

    def save_replied_tweets(self):
        """Lưu danh sách tweet đã reply ra file (nếu cần)"""
        try:
            # Nếu bạn muốn lưu ra file:
            # with open(f"replied_{self.profile_id}.json", "w", encoding="utf-8") as f:
            #     json.dump(list(self.replied_tweets), f, ensure_ascii=False)
            pass
        except Exception as e:
            self.log(f"Lỗi khi save replied_tweets: {str(e)}")


