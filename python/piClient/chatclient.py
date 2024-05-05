import asyncio
import websockets
import sys


async def send_and_receive(uri):
   async with websockets.connect(uri) as websocket:
       print("You can start typing your messages. Press Enter to send.")
       while True:
           message = await asyncio.get_event_loop().run_in_executor(None, sys.stdin.readline)
           message = message.strip()
           if message:
               await websocket.send(message)
               print(f"Sent: {message}")
              
               # Wait for the response from the server
               response = await websocket.recv()
               print(f"Received: {response}")


asyncio.get_event_loop().run_until_complete(send_and_receive('ws://192.168.132.130:6789'))