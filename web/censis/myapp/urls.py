from django.urls import path
from . import views
from .views import receive
app_name = "myapp"

urlpatterns = [
    path('',views.index, name = 'index'),
# The above maps any URLs starting with rango/ to be handled by rango.
    path("receive/", views.receive),
    path("add_device", views.index),
]
