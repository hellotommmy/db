select weight, count(*)
from col4tab1, col5tab1
where weight >= 20 and col4tab1.id = col5tab1.id
group by weight;









--select * from col4tab1, col5tab1;

--select * from col4tab1, col5tab1 where col4tab1.id = col5tab1.id;

--select * from col4tab1 where id > 10;
