import asyncio
import websockets
import smbus
import time


def send_data(bit):
   print(f"sending data: .{bit}.")
   bus = smbus.SMBus(1)
   bus.write_byte_data(0x12, 0, bit)
   bus.close()


async def echo(websocket, path):
       async for message in websocket:
           print(f"Received message: .{message}.")
           if message == "LedOn":
               send_data(0x01)
           if message == "LedOff":
               send_data(0x02)
           if message == "OpenDoor":
               send_data(0x03)


           print("!!Done writing")
           await websocket.send("GOT!")
           print("!!Sent back")


start_server = websockets.serve(echo, "192.168.132.130", 6789)


asyncio.get_event_loop().run_until_complete(start_server)
asyncio.get_event_loop().run_forever()
