-- 1. 
SELECT c.cat_name, COUNT(p.prod_id) AS product_count, avg(p.price) AS average_price
FROM categories c
LEFT JOIN products p ON c.cat_id = p.cat_id
GROUP BY c.cat_name;

-- 2. 

SELECT c.cat_name , SUM(oi.quantity * oi.unit_price) AS total_revenue
FROM categories c
JOIN products p ON c.cat_id = p.cat_id
JOIN order_items oi ON p.prod_id = oi.prod_id
GROUP BY c.cat_name
ORDER BY total_revenue DESC;

-- 3
SELECT cu.country, SUM(oi.quantity * oi.unit_price) AS total_revenue
FROM categories c
JOIN products p ON c.cat_id = p.cat_id
JOIN order_items oi ON p.prod_id = oi.prod_id
JOIN orders o ON oi.order_id = o.order_id
JOIN customers cu ON o.cust_id = cu.cust_id
GROUP BY cu.country;

-- 4

SELECT cu.cust_id,cu.name, COUNT(o.order_id) AS order_count
FROM customers cu
JOIN orders o ON cu.cust_id = o.cust_id
GROUP BY cu.cust_id, cu.name
HAVING COUNT(o.order_id) > 3
;

-- 5 
SELECT p.prod_id, p.prod_name, COALESCE(SUM(oi.quantity),0) AS total_quantity_sold
FROM products p
LEFT JOIN order_items oi ON p.prod_id = oi.prod_id
GROUP BY p.prod_id, p.prod_name;

-- 6 
SELECT 
    strftime('%Y-%m', o.order_date) AS order_month,
    SUM(oi.quantity * oi.unit_price) AS total_revenue
FROM orders o
JOIN order_items oi ON o.order_id = oi.order_id
 GROUP BY order_month
HAVING total_revenue > 15000;

-- 7 
SELECT c.cat_name, COUNT(DISTINCT p.prod_id) AS product_count, 
        AVG(p.price) AS average_price,
        COALESCE(SUM(oi.quantity),0) as total_units_sold,
        COALESCE(SUM(oi.quantity * oi.unit_price),0) as total_revenue
FROM categories c
LEFT JOIN products p ON c.cat_id = p.cat_id
LEFT JOIN order_items oi ON p.prod_id = oi.prod_id
GROUP BY c.cat_name;



-- Part 2 - subqueries

-- 8 

SELECT prod_id,prod_name 
FROM products
WHERE price > (SELECT AVG(price) FROM products);


-- 9 

SELECT p1.prod_id, p1.prod_name, p1.price
FROM products p1
WHERE p1.price > (SELECT AVG(p2.price) FROM products p2 WHERE p2.cat_id = p1.cat_id);

-- 10

-- NOT IN is risky when its subquery can return NULL: comparisons with NULL
-- become UNKNOWN, which can cause NOT IN to return no rows. NOT EXISTS is
-- NULL-safe because it tests whether a matching row exists.
SELECT cu.cust_id, cu.name
FROM customers cu
WHERE NOT EXISTS (
    SELECT 1
    FROM orders o
    WHERE o.cust_id = cu.cust_id
);

-- 11

SELECT cu.cust_id, cu.name
FROM customers cu
WHERE EXISTS (
    SELECT 1
    FROM orders o
    WHERE o.cust_id = cu.cust_id
      AND strftime('%Y', o.order_date) = '2023'
)
AND NOT EXISTS (
    SELECT 1
    FROM orders o
    WHERE o.cust_id = cu.cust_id
      AND strftime('%Y', o.order_date) = '2022'
);

-- 12

SELECT prod_id, prod_name, price
FROM products
WHERE price = (SELECT MAX(price) FROM products);

-- 13

-- Customers who have bought every product in the Books category.
SELECT cu.cust_id, cu.name
FROM customers cu
WHERE NOT EXISTS (
    SELECT 1
    FROM products p
    JOIN categories c ON c.cat_id = p.cat_id
    WHERE c.cat_name = 'Books'
      AND NOT EXISTS (
          SELECT 1
          FROM orders o
          JOIN order_items oi ON oi.order_id = o.order_id
          WHERE o.cust_id = cu.cust_id
            AND oi.prod_id = p.prod_id
      )
);

-- If no customer qualifies, this identifies the closest customer by counting
-- the distinct Books products they bought.
SELECT cu.cust_id, cu.name, COUNT(DISTINCT oi.prod_id) AS books_bought
FROM customers cu
JOIN orders o ON o.cust_id = cu.cust_id
JOIN order_items oi ON oi.order_id = o.order_id
JOIN products p ON p.prod_id = oi.prod_id
JOIN categories c ON c.cat_id = p.cat_id
WHERE c.cat_name = 'Books'
GROUP BY cu.cust_id, cu.name
ORDER BY books_bought DESC
LIMIT 1;

-- 14

WITH cust_revenue AS (
    SELECT o.cust_id, SUM(oi.quantity * oi.unit_price) AS revenue
    FROM orders o
    JOIN order_items oi ON oi.order_id = o.order_id
    GROUP BY o.cust_id
), ranked_customers AS (
    SELECT cu.country, cu.cust_id, cu.name, cr.revenue,
           RANK() OVER (PARTITION BY cu.country ORDER BY cr.revenue DESC) AS revenue_rank
    FROM customers cu
    JOIN cust_revenue cr ON cr.cust_id = cu.cust_id
)
SELECT country, cust_id, name, revenue
FROM ranked_customers
WHERE revenue_rank = 1;

-- 15

-- prerequisite relation prereq(course_id, prereq_id).
-- this is the query for the university course prerequisites problem. It finds all the prerequisites for a given course, including indirect prerequisites.
WITH RECURSIVE prerequisites(course_id, prereq_id) AS (
    SELECT course_id, prereq_id
    FROM prereq
    WHERE course_id = 'CS-347'
    UNION
    SELECT p.course_id, p.prereq_id
    FROM prereq p
    JOIN prerequisites r ON p.course_id = r.prereq_id
)
SELECT DISTINCT prereq_id
FROM prerequisites;

