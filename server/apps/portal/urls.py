from django.urls import path
from apps.portal.views import *

app_name = "portal"

urlpatterns = [

    path('',IndexView.as_view(), name="index"),
    path('dashboard/',DashboardView.as_view(), name="dashboard"),

]