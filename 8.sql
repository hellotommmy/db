select frequency, max(id)
from col5tab1
where frequency >= 40
group by frequency;
