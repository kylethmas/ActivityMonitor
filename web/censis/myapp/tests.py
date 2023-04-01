from django.test import TestCase
from myapp.models import Device, Alert
import datetime
from django.urls import reverse
from populate_script import populate,add_device,add_alert



# Create your tests here.
class DeviceMethodTests(TestCase):
    """ Test Device model"""

    def test_deviceid_not_null(self):
        """
        Ensures the deviceID is a non negative number.
        """
        device = Device(deviceName = "test1")
        device.save()
        self.assertEqual((device.deviceID >= 0), True)

    def test_name_creation(self):
        """
        Checks to make sure that when a device is created, an
        appropriate name is created.
        """
        device = add_device("test2")
        self.assertEqual(device.deviceName, 'test2')

    def test_location_creation(self):
        """
        Checks to make sure that when a device is created, an
        appropriate location is created.
        """
        device = add_device("test3")
        self.assertEqual(device.location, "Hallway")

    def test_person_creation(self):
        """
        Checks to make sure that when a device is created, an
        appropriate devicePerson is created.
        """
        device = add_device("test4")
        self.assertEqual(device.devicePerson, "jean")

    def test_str_method(self):
        """
        Checks to make sure the __str__ method works.
        """
        device = add_device("test5")
        self.assertEqual(str(device), "test5")

def add_alert(noise,dev):
    """ Helper to add a alert """
    alert = Alert(noise=noise, device = dev)
    alert.save()
    return alert

class AlertMethodTests(TestCase):
    """tests Alert Model"""
    def test_alerteid_not_null(self):
        """
        Ensures the alertID is a non negative number
        """
        device = add_device("test6")
        alert = add_alert("Shout",device)
        self.assertEqual((alert.alertID >= 0), True)

    def test_noise_creation(self):
        """
        Checks to make sure that when a alert is created, an
        appropriate name is created.
        """
        device = add_device("test7")
        alert = add_alert("Shout",device)

        self.assertEqual(alert.noise, "Shout")
 
    def test_device_creation(self):
        """
        Checks to make sure that when a alert is created, an
        appropriate device is related.
        """
        device = add_device("test8")
        alert = add_alert("Shout",device)

        self.assertEqual(alert.device, device)

    def test_timestamp_creation(self):
        """
        Checks to make sure that when a alert is created, an
        appropriate timestamp object is created to represent the time created.
        """
        device = add_device("test9")
        alert = add_alert("Shout",device)

        self.assertEqual(type(alert.created), datetime.datetime)

    def test_str_method(self):
        """
        Checks to make sure the __str__ method works.
        """
        device = add_device("test10")
        alert = add_alert("Shout",device)
        self.assertEqual(str(alert), "Shout")


class IndexViewTests(TestCase):

    def test_index_view_with_no_devices(self):
        """
        If no devices exist, the appropriate message should be displayed.
        """
        response = self.client.get(reverse('myapp:index'))
        self.assertEqual(response.status_code, 200)
        self.assertContains(response, 'Add a device to see data')
        self.assertQuerysetEqual(response.context['devices_list'], [])

    def test_index_view_with_devices_but_no_alerts(self):
        """
        If no alerts exist, the appropriate message should be displayed.
        """
        add_device("device1")
        add_device("device2")
        add_device("device3")
        response = self.client.get(reverse('myapp:index'))
        self.assertEqual(response.status_code, 200)
        self.assertContains(response, 'device1')
        self.assertContains(response, 'device2')
        self.assertContains(response, 'device3')

        self.assertContains(response, 'Device data will go here.')

        num_devices = len(response.context['devices_list'])
        self.assertEquals(num_devices, 3)

    def test_index_view_with_one_device_and_one_alert(self):
        """ If device is added and alert is added associated with device,
        both are displayed """
        dev = add_device("device1")
        add_alert("dog",dev)

        response = self.client.get(reverse('myapp:index'))
        self.assertEqual(response.status_code, 200)
        self.assertContains(response, 'device1')
        self.assertContains(response, 'dog')

    def test_index_view_with_devices_with_alerts_and_empty_devices(self):
        """ displays correct display for all devices """

        dev = add_device("device1")
        add_alert("dog",dev)
        add_alert("shout",dev)
        add_device("device2")

        response = self.client.get(reverse('myapp:index'))
        self.assertEqual(response.status_code, 200)
        self.assertContains(response, 'device1')
        self.assertContains(response, 'dog')
        self.assertContains(response, 'shout')
        self.assertContains(response, 'device2')
        self.assertContains(response, 'Device data will go here.')

    def test_add_device_form_working(self):
        """ tests add device form is working as expected"""
        response = self.client.post(reverse('myapp:index'),
                                    {'deviceName' : 'devNiamh',
                                     'location' : 'bedroom', 'devicePerson':'Niamh','submit':''})
        self.assertEqual(response.status_code, 302)
        self.assertEqual(len(Device.objects.all()), 1)

    def test_add_device_form_not_working(self):
        """ tests add device form can handle unexpected input"""
        response = self.client.post(reverse('myapp:index'),
                                    {'deviceName' : '',
                                     'location' : 'bedroom', 'devicePerson':'Niamh'})

        self.assertEqual(response.status_code, 200)
        self.assertEqual(len(Device.objects.all()), 0)

class ReceiveViewTests(TestCase):
    """ Class to test receive function """

    def test_receive_view_working(self):
        """Function to test that receive works"""
        response = self.client.post(reverse('myapp:receive'),
                                    {'api_key' : 'tPmAT5Ab3j7F9',
                                     'sensor' : 'device2', 'value':'TEST1'})
        self.assertEqual(response.status_code, 200)
        self.assertEqual(len(Alert.objects.all()), 1)
        self.assertContains(response, 'POST request processed successfully')

    def test_receive_view_with_no_apikey_provided(self):
        """ tests receive view with no apikey"""
        response = self.client.post(reverse('myapp:receive'),
                                    {'sensor' : 'device2', 'value':'TEST2'})
        self.assertEqual(len(Alert.objects.all()), 0)
        self.assertContains(response, 'Invalid API Key')
  
    def test_receive_view_with_get_method(self):
        """ tests recceieve view with get method"""
        response = self.client.get(reverse('myapp:receive'),
                                    {'api_key' : 'tPmAT5Ab3j7F9',
                                     'sensor' : 'device2', 'value':'TEST3 '})
        self.assertEqual(len(Alert.objects.all()), 0)
        self.assertContains(response, 'Only POST requests are allowed')


class PopulationTests(TestCase):
    """ class to test population script"""
    def test_add_device(self):
        """ tests add device function works """
        add_device("test","room","jep")
        self.assertEqual(len(Device.objects.all()), 1)

    def test_add_alert(self):
        """ tests add alert function works """
        dev = add_device("test","room","jep")
        add_alert("noiseTest",dev)
        self.assertEqual(len(Alert.objects.all()), 1)

    def population_test(self):
        """ Population script test"""
        populate()
        self.assertEqual(len(Device.objects.all()), 3)
        self.assertEqual(len(Alert.objects.all()), 15)
