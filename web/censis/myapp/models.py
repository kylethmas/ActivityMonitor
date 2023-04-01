# pylint: disable=too-few-public-methods
""" module providing models support"""
from django.db import models

# Create your models here.

class Device(models.Model):
    """ class representing a Device"""
    deviceID = models.BigAutoField(primary_key=True)
    deviceName = models.CharField(max_length=128, unique=True)
    location = models.CharField(max_length=128, null = True)
    devicePerson = models.CharField(max_length = 128, null = True)
    def __str__(self):
        return self.deviceName

class Alert(models.Model):
    """ class representing an alert """
    alertID = models.BigAutoField(primary_key=True)
    noise = models.CharField(max_length=128)
    device = models.ForeignKey(Device, on_delete=models.CASCADE)
    created = models.DateTimeField(auto_now_add=True)
    def __str__(self):
        return self.noise
    