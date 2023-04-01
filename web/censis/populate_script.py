"""Module to populate dataabse"""
import os
os.environ.setdefault('DJANGO_SETTINGS_MODULE', 'censis.settings')
import django
django.setup()
from myapp.models import Device, Alert

def populate():
    """Function to populate"""
    devices = [
        {'name':'device1',
         'location': 'kitchen',
         'devicePerson': 'Sarah' },
        {'name':'device2',
         'location': 'hallway',
         'devicePerson': 'Jean' },
        {'name':'device3',
         'location': 'bedroom',
         'devicePerson': 'Bob' }
    ]

    alerts = [
        {'Noise':'Alarm',
         'device' : 'device1'},
         {'Noise':'Barking',
         'device' : 'device1'},
         {'Noise':'Speech',
         'device' : 'device1'},
         {'Noise':'Alarm',
         'device' : 'device1'},
         {'Noise':'Toilet',
         'device' : 'device2'},
         {'Noise':'Car',
         'device' : 'device2'},
         {'Noise':'Music',
         'device' : 'device2'},
         {'Noise':'Microwave',
         'device' : 'device2'},
         {'Noise':'Speech',
         'device' : 'device2'},
         {'Noise':'Singing',
         'device' : 'device3'},
         {'Noise':'Children',
         'device' : 'device3'},
         {'Noise':'Animal',
         'device' : 'device3'},
         {'Noise':'Alarm',
         'device' : 'device3'},
         {'Noise':'Speech',
         'device' : 'device3'},
         {'Noise':'Shouts',
         'device' : 'device3'},
    ]

    for dev in devices:
        add_device(dev["name"],dev["location"],dev["devicePerson"])
    for alert in alerts:
        add_alert(alert["Noise"], alert["device"]).alertID


def add_device(device_name="test",loc="Hallway",per="jean"):
    """Helper Function to add a device object"""
    device = Device.objects.get_or_create(deviceName = device_name)[0]
    device.location = loc
    device.devicePerson = per
    device.save()
    return device

def add_alert(noise, device_name):
    """Function to add a alert object"""
    device = Device.objects.get(deviceName = device_name)
    alert = Alert()
    alert.noise = noise
    alert.device = device
    alert.save()
    return alert

if __name__ == '__main__':
    print('Starting population script...')
    populate()
