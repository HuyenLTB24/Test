#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QString>

namespace TwitterConstants {
    // Status strings
    static const QString STATUS_SUCCESS = "Thành công";
    static const QString STATUS_FAILED = "Thất bại";
    
    // CSS Selectors for Twitter elements
    static const QString SELECTOR_TWEET = "article[data-testid=\"tweet\"]";
    static const QString SELECTOR_TWEET_TEXT = "[data-testid=\"tweetText\"]";
    static const QString SELECTOR_REPLY_BUTTON = "[data-testid=\"reply\"]";
    static const QString SELECTOR_TWEET_BUTTON = "[data-testid=\"tweetButton\"]";
    static const QString SELECTOR_TWEET_TEXTAREA = "[data-testid=\"tweetTextarea_0\"]";
    static const QString SELECTOR_FOLLOW_BUTTON = "[data-testid$=\"-follow\"], [data-testid$=\"-unfollow\"]";
    static const QString SELECTOR_LIKE_BUTTON = "[data-testid=\"like\"]";
    static const QString SELECTOR_USER_NAME = "[data-testid=\"User-Name\"]";
    static const QString SELECTOR_VERIFIED_ICON = "[data-testid=\"icon-verified\"]";
    static const QString SELECTOR_VERIFIED_BADGE = "[aria-label*=\"Verified\"]";
    
    // Bot modes
    enum class BotMode {
        FEED_MODE = 1,
        USER_MODE = 2,
        COMMENTS_MODE = 3,
        TRENDING_MODE = 4
    };
    
    // Log levels
    enum class LogLevel {
        INFO,
        WARNING,
        ERROR,
        DEBUG,
        SUCCESS
    };
    
    // Default settings
    static const int DEFAULT_MAX_REPLIES = 50;
    static const int DEFAULT_MIN_VIEWS = 0;
    static const int DEFAULT_INTERVAL = 30;
    static const int DEFAULT_REPLY_INTERVAL = 0;
    static const int DEFAULT_TIME_LIMIT_HOURS = 24;
    static const int DEFAULT_TIME_LIMIT_MINUTES = 0;
}

#endif // CONSTANTS_H
