""" Module which displays views"""
from django.shortcuts import render, redirect
from myapp.models import Alert, Device
from myapp.forms import DeviceForm
from django.http import HttpResponse
from django.views.decorators.csrf import csrf_exempt
from django.http import JsonResponse
from django.conf import settings


# Create your views here.

def index(request):
    """Index view function"""
    devices_list = Device.objects.all()
    devices = {}
    for device in devices_list:
        alerts = Alert.objects.filter(device=device)
        devices[device] = alerts
    context_dict = {}
    context_dict['devices_list'] = devices_list
    context_dict['devices'] = devices

    form = DeviceForm()
    
    # If the form is submitted, process it. Adds a new device
    if request.method == 'POST':
        if 'submit' in request.POST:
            form = DeviceForm(request.POST)
            if form.is_valid():
                form.save(commit=True)
                # Now that the device is saved, we could confirm this.
                # For now, just redirect the user back to the index view.
                return redirect('/')

            # The supplied form contained errors
            print(form.errors)
    context_dict['form'] = form
    return render(request,'myapp/index.html',context=context_dict)

# Receive data from the sensor
@csrf_exempt # This is required to allow POST requests from the sensor
def receive(request): 
    if request.method == 'POST':
        api_key = request.POST.get('api_key', None)
        if api_key is not None:
            sensor_name = request.POST.get('sensor', '')
            value = request.POST.get('value', '')
            device, _ = Device.objects.get_or_create(deviceName=sensor_name)
            alert = Alert(noise=value, device=device)
            alert.save()
            return JsonResponse({'message': 'POST request processed successfully.'}) 
        else:
            return JsonResponse({'message': 'Invalid API Key.'})
    else:
        return JsonResponse({'message': 'Only POST requests are allowed.'})


