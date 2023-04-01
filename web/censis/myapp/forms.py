from django import forms
from myapp.models import Device

class DeviceForm(forms.ModelForm):
    deviceName = forms.CharField(max_length=128,help_text="Please enter the device name e.g. device9.")
    location = forms.CharField(max_length=128,help_text="Please enter the location e.g. kitchen.")
    devicePerson = forms.CharField(max_length=128,help_text="Please enter the person's name e.g. Jean.")

    class Meta:
        # Provide an association between the ModelForm and a model
        model = Device
        fields = ('deviceName','location','devicePerson')


