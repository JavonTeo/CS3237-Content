from flask import Flask, request, render_template, Response

app = Flask(__name__)

# Create just a single route to read data from our ESP32
@app.route('/data', methods = ['GET'])
def addData():
    ''' The one and only route. It extracts the
    data from the request, converts to float if the
    data is not None, then calls the callback if it is set
    '''
    global _callback_

    data1Str = request.args.get('data1')
    data2Str = request.args.get('data2')

    data1 = float(data1Str) if data1Str else None
    data2 = float(data2Str) if data2Str else None

    print("\nData from ESP32: ", data1, data2, "\n")
    
    return "OK", 200

def main():
    app.run(host = "0.0.0.0", port = '3237')


if __name__ == '__main__':
    main()
