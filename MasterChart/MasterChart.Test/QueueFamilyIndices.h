#pragma once

struct QueueFamilyIndices
{
    int GraphFamily = -1;
    int PresentFamily = -1;

    bool IsComplete() const
    {
        return GraphFamily >= 0 && PresentFamily >= 0;
    }
};
