from flask import Flask, jsonify, request
from twilio.rest import Client
from dotenv import load_dotenv
import os
import mysql.connector

app = Flask(__name__)

load_dotenv()

# Retrieve Twilio credentials from environment variables
account_sid = os.environ.get("TWILIO_ACCOUNT_SID")
auth_token = os.environ.get("TWILIO_AUTH_TOKEN")
twilio_client = Client(account_sid, auth_token)

def send_whatsapp_message(to_phone, code):
    from_whatsapp_number = "whatsapp:+@@@@@@@@@"  # Twilio Sandbox WhatsApp number
    to_whatsapp_number = f"whatsapp:{to_phone}"      # The recipient's WhatsApp number

    try:
        message = twilio_client.messages.create(
            body=f"Your security code is: {code}",
            from_=from_whatsapp_number,
            to=to_whatsapp_number
        )
        print(f"Message sent: {message.sid}")
        return True
    except Exception as e:
        print(f"Error: {e}")
        return False

def get_sensor_data():
    try:
        conn = mysql.connector.connect(
            host='@@@@@@@@@',
            user='@@@@@@@@@@',
            password='@@@@@@@@@',
            database='@@@@@@@@@@'
        )
        cursor = conn.cursor(dictionary=True)
        cursor.execute("SELECT * FROM sensor_data")
        rows = cursor.fetchall()
        conn.close()
        return rows
    except mysql.connector.Error as err:
        print(f"Error: {err}")
        return []

@app.route('/sensor_data', methods=['GET'])
def sensor_data():
    data = get_sensor_data()
    return jsonify(data)

@app.route('/send_code', methods=['POST'])
def send_code():
    data = request.json
    phone_number = data.get('phoneNumber')  # Make sure your request JSON uses 'phoneNumber'
    code = data.get('code')
    
    if phone_number and code:  # Check if both phoneNumber and code are provided
        if send_whatsapp_message(phone_number, code):
            return jsonify({"message": "Security code sent successfully"}), 200
        else:
            return jsonify({"error": "Failed to send security code"}), 500
    return jsonify({"error": "Phone number and code are required"}), 400

if __name__ == '__main__':
    app.run(host='0.0.0.0')
