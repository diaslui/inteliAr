from django.shortcuts import render
from django.views.generic import TemplateView


class IndexView(TemplateView):
    template_name = 'portal/index.html'

    def get_context_data(self, **kwargs):
        context = super().get_context_data(**kwargs)
        return context
    

class DashboardView(TemplateView):
    template_name = 'portal/dashboard.html'

    def get_context_data(self, **kwargs):
        context = super().get_context_data(**kwargs)
        return context