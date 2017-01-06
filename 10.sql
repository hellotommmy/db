select weight, count(*)
from col4tab1, col5tab1
where weight >= 20 and col4tab1.id = col5tab1.id
group by weight;
