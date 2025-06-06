# X2 Modern - Twitter Bot with Modern UI

![X2 Modern Logo](resources/icon.ico)

X2 Modern is an advanced Twitter automation tool with a sleek, modern interface built with PyQt6. This application lets users manage multiple Twitter accounts, automate interactions, and generate AI-powered content using ChatGPT or Gemini.

<p align="center">
  <a href="#key-features">Features</a> •
  <a href="#installation">Installation</a> •
  <a href="#usage">Usage</a> •
  <a href="#screenshots">Screenshots</a> •
  <a href="#advanced-features">Advanced</a> •
  <a href="#license">License</a>
</p>

## Key Features

- **Modern Dashboard**: Clean, intuitive UI with real-time activity monitoring
- **Multi-Account Management**: Add, edit, delete, and manage multiple Twitter accounts
- **Multiple Automation Modes**:
  - **Feed Mode**: Monitor timeline and automatically interact with tweets
  - **User Mode**: Follow and interact with specific users' tweets
  - **Comments Mode**: Automatically respond to users who comment on your tweets
  - **Trending Mode**: Find and rewrite tweets based on trending keywords
- **AI Integration**: Use Gemini or ChatGPT to generate intelligent responses
- **Browser Management**: Connect with Chrome via remote debugging protocol
- **Statistics & Reporting**: Track activities and performance in real-time
- **Advanced Settings**: Fine-tune parameters to optimize bot performance
- **Proxy Support**: Compatible with Multilogin and other browser profile management tools

## Installation

### Requirements

- Python 3.8 or higher
- Chrome Browser (latest version recommended)
- Twitter/X account(s)
- Multilogin or similar browser profile management tool (recommended)

### Setup Instructions

1. Clone the repository
   ```powershell
   git clone https://github.com/yourusername/X2_Modern.git
   cd X2_Modern
   ```

2. Install dependencies
   ```powershell
   pip install -r requirements.txt
   ```

3. Run the application
   ```powershell
   python src/twitter_bot_gui.py
   ```

### Build Executable (Optional)

To create a standalone executable:

```powershell
python build_exe.py
```

The executable will be available in the `build/X2_Modern` directory.

## Usage

### Initial Setup

1. **Add Twitter Accounts**: 
   - Open the "Account Manager" from the menu
   - Add your Twitter account credentials
   - Configure proxy settings if needed

2. **Configure AI API**:
   - Enter your API keys for Gemini or ChatGPT
   - Customize AI response parameters

3. **Adjust Bot Settings**:
   - Set interaction frequencies
   - Configure content filters
   - Customize automation behavior

### Running the Bot

1. Select an account from the left sidebar
2. Choose an operation mode (Feed, User, Comments, or Trending)
3. Configure mode-specific settings
4. Click "Start" to begin automation
5. Monitor activity through the "Activity" and "Logs" tabs

## Screenshots

[Screenshots would be placed here]

## Advanced Features

### Custom Interaction Rules

Create custom rules to determine how the bot interacts with specific content:

- Keyword-based filtering
- Sentiment analysis integration
- User reputation scoring
- Content category detection

### AI Response Customization

Fine-tune AI-generated responses with:

- Custom personality profiles
- Topic specialization
- Language style adjustment
- Response length control

### Automation Scheduling

Schedule bot activities based on:

- Time of day optimization
- Day of week preferences
- Target audience activity patterns

## Project Structure

```
X2_Modern/
├── build_exe.py          # Executable build script
├── config/               # Configuration files
│   └── config.json       # Application settings
├── resources/            # UI resources
│   ├── icon.ico          # Application icon
│   └── style.qss         # Qt stylesheet
├── src/                  # Source code
│   ├── account_manager.py  # Account management
│   ├── api_manager.py    # AI API integration
│   ├── bot_manager.py    # Bot control logic
│   ├── browser.py        # Browser automation
│   ├── database.py       # Database operations
│   ├── twitter_bot.py    # Core Twitter functionality
│   ├── twitter_bot_gui.py# Main GUI application
│   ├── ui/               # UI components
│   │   ├── dialogs.py    # Dialog windows
│   │   └── widgets.py    # Custom widgets
│   └── utils/            # Utility functions
│       ├── logger.py     # Logging system
│       └── text_utils.py # Text processing utilities
├── requirements.txt      # Python dependencies
└── README.md             # Project documentation
```

## Notes

- This application requires Chrome and potentially Multilogin for profile management
- API keys for Gemini/ChatGPT must be provided for full functionality
- Usage must comply with Twitter/X's Terms of Service
- For educational purposes only - use responsibly

## License

This project is licensed under the [MIT License](LICENSE)

---

**Disclaimer**: This tool is for educational purposes. Users are responsible for ensuring their usage complies with Twitter/X's Terms of Service.