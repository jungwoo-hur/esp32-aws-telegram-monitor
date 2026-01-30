from flask import Flask
from flask import request
import requests

app = Flask(name)


TOKEN = "7809756***:AAEOVQVZGMoG******************"

def get_chat_id():
    url = f"https://api.telegram.org/bot%7BTOK*****/getUpdates"
    response = requests.get(url)
    if response.ok:
        updates = response.json()
        if updates["result"]:
            return updates["result"][0]["message"]["chat"]["id"]
    return None

def send_telegram_message(message):
    chat_id = get_chat_id()
    if chat_id:
        url = f"https://api.telegram.org/bot%7BTOK*****/sendMessage"
        data = {
            "chat_id": chat_id,
            "text": message
        }
        try:
            requests.post(url, json=data)
        except Exception as e:
            print(f"Error sending message: {e}")

@app.route("/")
def hello():
    value = request.args.get("var")
    print(value)

    if value:
        send_telegram_message(f"update: {value}")

    return "We received value: " + str(value)
