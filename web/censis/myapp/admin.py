"""Module providing admin function"""
from django.contrib import admin
from myapp.models import Device, Alert

# Register your models here.
admin.site.register(Device)
admin.site.register(Alert)
